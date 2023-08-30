/*
   Copyright 2022-2023 Nora Beda

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;

namespace LibChess
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Move
    {
        public Coord Position;
        public Coord Destination;
    }

    public struct PieceQuery
    {
        public PieceType? Type { get; set; }
        public PlayerColor? Color { get; set; }
        public int? X { get; set; }
        public int? Y { get; set; }
        public Func<PieceInfo, bool>? Filter { get; set; }

        public PieceQuery()
        {
            Type = null;
            Color = null;
            X = null;
            Y = null;
            Filter = null;
        }
    }

    public sealed class Engine : IDisposable, IEquatable<Engine>
    {
        private static readonly IReadOnlyList<PieceType> sValidPieceTypes;
        static Engine()
        {
            sValidPieceTypes = new PieceType[]
            {
                PieceType.Queen,
                PieceType.Rook,
                PieceType.Knight,
                PieceType.Bishop
            };
        }

        public unsafe Engine()
        {
            mAddress = NativeFunctions.CreateEngine();

            NativeFunctions.EngineCaptureCallback callback = OnPieceCapture;
            NativeFunctions.SetEngineCaptureCallback(mAddress, callback);
            mCallbackHandle = GCHandle.Alloc(callback);

            mDisposed = false;
            mLastMove = null;
        }

        ~Engine()
        {
            if (!mDisposed)
            {
                Dispose(false);
            }
        }

        public void Dispose()
        {
            if (mDisposed)
            {
                return;
            }

            Dispose(true);
            mDisposed = true;
        }

        private void Dispose(bool disposing)
        {
            NativeFunctions.DestroyEngine(mAddress);
            mCallbackHandle.Free();
        }

        private bool IsPromotionMove(Move move, bool committed)
        {
            using var board = Board;
            if (board is null)
            {
                return false;
            }

            board.GetPiece(committed ? move.Destination : move.Position, out PieceInfo piece);
            if (piece.Type != PieceType.Pawn)
            {
                return false;
            }

            int destinationRank = piece.Color == PlayerColor.White ? 7 : 0;
            return move.Destination.Y == destinationRank;
        }

        public bool ShouldPromote(PlayerColor player, bool committed = true)
        {
            if (mLastMove is null)
            {
                return false;
            }

            var lastMove = mLastMove.Value;
            if (!IsPromotionMove(lastMove, committed))
            {
                return false;
            }

            using var board = Board;
            if (board is null || !board.GetPiece(lastMove.Destination, out PieceInfo piece))
            {
                return false;
            }

            return piece.Color == player;
        }

        public Board? Board
        {
            get
            {
                var address = NativeFunctions.GetEngineBoard(mAddress);
                if (address != IntPtr.Zero)
                {
                    return new Board(address);
                }
                else
                {
                    return null;
                }
            }
            set
            {
                NativeFunctions.SetEngineBoard(mAddress, value?.mAddress ?? IntPtr.Zero);
                mLastMove = null;
            }
        }

        public event Action<PlayerColor>? Check;
        public event Action<PlayerColor>? Checkmate;
        public event Action<PieceInfo>? PieceCaptured;

        private unsafe void OnPieceCapture(PieceInfo* piece) => PieceCaptured?.Invoke(*piece);

        public unsafe IReadOnlyList<Coord> FindPieces(PieceQuery query)
        {
            var ptr = NativeFunctions.CreatePieceQuery();

            if (query.Type != null)
            {
                NativeFunctions.SetQueryPieceType(ptr, query.Type.Value);
            }

            if (query.Color != null)
            {
                NativeFunctions.SetQueryPieceColor(ptr, query.Color.Value);
            }

            if (query.X != null)
            {
                NativeFunctions.SetQueryPieceX(ptr, query.X.Value);
            }

            if (query.Y != null)
            {
                NativeFunctions.SetQueryPieceY(ptr, query.Y.Value);
            }

            if (query.Filter != null)
            {
                NativeFunctions.SetQueryFilter(ptr, piece => query.Filter(*piece));
            }

            var pieces = new List<Coord>();
            NativeFunctions.EngineFindPieces(mAddress, ptr, position => pieces.Add(*position));

            return pieces;
        }

        public unsafe bool ComputeCheck(PlayerColor color, IList<Coord>? pieces)
        {
            return NativeFunctions.EngineComputeCheck(mAddress, color, position => pieces?.Add(*position));
        }

        public bool ComputeCheckmate(PlayerColor color) => NativeFunctions.EngineComputeCheckmate(mAddress, color);

        public unsafe IReadOnlyList<Coord> ComputeLegalMoves(Coord position)
        {
            var moves = new List<Coord>();
            NativeFunctions.EngineComputeLegalMoves(mAddress, &position, dest => moves.Add(*dest));

            return moves;
        }

        public unsafe bool IsMoveLegal(Move move)
        {
            return NativeFunctions.EngineIsMoveLegal(mAddress, &move);
        }

        public unsafe bool CommitMove(Move move)
        {
            using var board = Board;
            if (board is null)
            {
                throw new InvalidOperationException("No board exists!");
            }

            if (ShouldPromote(board.CurrentTurn, true))
            {
                return false;
            }

            bool isPromotionMove = IsPromotionMove(move, false);
            if (!NativeFunctions.EngineCommitMove(mAddress, &move, !isPromotionMove))
            {
                return false;
            }

            if (!board.GetPiece(move.Destination, out PieceInfo piece))
            {
                throw new Exception("Could not find the piece at the move destination!");
            }

            var oppositeColor = piece.Color != PlayerColor.White ? PlayerColor.White : PlayerColor.Black;
            if (ComputeCheckmate(oppositeColor))
            {
                Checkmate?.Invoke(oppositeColor);
            }
            else if (ComputeCheck(oppositeColor, null))
            {
                Check?.Invoke(oppositeColor);
            }

            mLastMove = move;
            return true;
        }

        public bool Promote(PieceType type)
        {
            using var board = Board;
            if (board is null)
            {
                throw new InvalidOperationException("No board exists!");
            }

            if (!ShouldPromote(board.CurrentTurn))
            {
                return false;
            }

            if (!sValidPieceTypes.Contains(type))
            {
                return false;
            }

            var lastMove = mLastMove!.Value;
            if (!board.GetPiece(lastMove.Destination, out PieceInfo piece))
            {
                throw new Exception("Could not find the piece to promote!");
            }

            piece.Type = type;
            board.SetPiece(lastMove.Destination, piece);

            board.AdvanceTurn();
            return true;
        }

        public void ClearCache() => NativeFunctions.ClearEngineCache(mAddress);

        public override int GetHashCode() => mAddress.GetHashCode();
        public override bool Equals(object? obj)
        {
            if (obj is Engine other)
            {
                return Equals(other);
            }
            else
            {
                return false;
            }
        }

        public bool Equals(Engine? other)
        {
            if (other is not null)
            {
                return mAddress == other.mAddress;
            }
            else
            {
                return false;
            }
        }

        public static bool operator ==(Engine? lhs, Engine? rhs) => lhs?.Equals(rhs) ?? (rhs is null);
        public static bool operator !=(Engine? lhs, Engine? rhs) => !(lhs == rhs);

        private readonly IntPtr mAddress;
        private readonly GCHandle mCallbackHandle;
        public bool mDisposed;
        private Move? mLastMove;
    }
}