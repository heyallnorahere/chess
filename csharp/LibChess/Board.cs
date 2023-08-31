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
using System.Runtime.InteropServices;

namespace LibChess
{
    public enum PieceType : byte
    {
        None = 0,
        King,
        Queen,
        Rook,
        Knight,
        Bishop,
        Pawn
    }

    [AttributeUsage(AttributeTargets.Field, AllowMultiple = false, Inherited = false)]
    public sealed class PlayerColorValueAttribute : Attribute
    {
        public PlayerColorValueAttribute(byte value)
        {
            Value = value;
        }

        public byte Value { get; }
    }

    public enum PlayerColor : byte
    {
        [PlayerColorValue(255)]
        White = 0,
        
        [PlayerColorValue(0)]
        Black
    }

    [Flags]
    public enum CastleFlags : byte
    {
        None = 0,
        KingSide = (1 << 0),
        QueenSide = (1 << 1)
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct PieceInfo
    {
        public PieceType Type;
        public PlayerColor Color;
    }

    public sealed class Board : IDisposable, IEquatable<Board>
    {
        public const int Width = 8;

        public static Board Create()
        {
            var address = NativeFunctions.CreateBoardDefault();
            return new Board(address);
        }

        public static Board? Create(string fen)
        {
            var address = NativeFunctions.CreateBoard(fen);
            if (address == IntPtr.Zero)
            {
                return null;
            }

            return new Board(address);
        }

        internal Board(IntPtr address)
        {
            mAddress = address;
        }

        ~Board() => Dispose(false);

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        private void Dispose(bool disposing)
        {
            NativeFunctions.DestroyBoard(mAddress);
        }

        public override int GetHashCode() => mAddress.GetHashCode();

        public override bool Equals(object? obj)
        {
            if (obj is Board other)
            {
                return Equals(other);
            }
            else
            {
                return false;
            }
        }

        public bool Equals(Board? other)
        {
            if (other is not null)
            {
                var lhs = NativeFunctions.GetInternalBoardPointer(mAddress);
                var rhs = NativeFunctions.GetInternalBoardPointer(other.mAddress);
                return lhs == rhs;
            }
            else
            {
                return false;
            }
        }

        public static unsafe bool IsOutOfBounds(Coord position) => NativeFunctions.IsOutOfBounds(&position);

        public unsafe bool GetPiece(Coord position, out PieceInfo piece)
        {
            var tempPiece = new PieceInfo();
            if (NativeFunctions.GetBoardPiece(mAddress, &position, &tempPiece))
            {
                piece = tempPiece;
                return true;
            }
            else
            {
                piece = new PieceInfo
                {
                    Type = PieceType.None,
                    Color = PlayerColor.White
                };

                return false;
            }
        }

        public unsafe bool SetPiece(Coord position, PieceInfo piece) => NativeFunctions.SetBoardPiece(mAddress, &position, &piece);
        public unsafe string SerializeFEN()
        {
            var ptr = NativeFunctions.SerializeBoardFEN(mAddress);
            var fen = Marshal.PtrToStringAnsi(ptr) ?? throw new ArgumentException();

            NativeFunctions.FreeMemory(ptr);
            return fen;
        }

        public void AdvanceTurn() => NativeFunctions.AdvanceTurn(mAddress);

        public PlayerColor CurrentTurn => NativeFunctions.GetCurrentBoardTurn(mAddress);
        public CastleFlags GetCastlingAvailability(PlayerColor player) => NativeFunctions.GetBoardCastlingAvailability(mAddress, player);

        public unsafe Coord? EnPassantTarget
        {
            get
            {
                var target = new Coord();
                if (NativeFunctions.GetBoardEnPassantTarget(mAddress, &target))
                {
                    return target;
                }
                else
                {
                    return null;
                }
            }
        }

        public ulong HalfmoveClock => NativeFunctions.GetBoardHalfmoveClock(mAddress);
        public ulong FullmoveCount => NativeFunctions.GetBoardFullmoveCount(mAddress);

        public static bool operator ==(Board? lhs, Board? rhs) => lhs?.Equals(rhs) ?? (lhs is null);
        public static bool operator !=(Board? lhs, Board? rhs) => !(lhs == rhs);

        internal readonly IntPtr mAddress;
    }
}