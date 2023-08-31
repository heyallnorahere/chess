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
    internal static class NativeFunctions
    {
        private const string sNativeLibraryName = "LibChess.Native";

        #region Board

        [DllImport(sNativeLibraryName)]
        public static extern IntPtr CreateBoardDefault();

        [DllImport(sNativeLibraryName)]
        public static extern IntPtr CreateBoard(string fen);

        [DllImport(sNativeLibraryName)]
        public static extern void DestroyBoard(IntPtr address);

        [DllImport(sNativeLibraryName)]
        public static extern unsafe bool IsOutOfBounds(Coord* position);

        [DllImport(sNativeLibraryName)]
        public static extern unsafe bool GetBoardPiece(IntPtr address, Coord* position, PieceInfo* piece);

        [DllImport(sNativeLibraryName)]
        public static extern unsafe bool SetBoardPiece(IntPtr address, Coord* position, PieceInfo* piece);

        [DllImport(sNativeLibraryName)]
        public static extern IntPtr SerializeBoardFEN(IntPtr address);

        [DllImport(sNativeLibraryName)]
        public static extern void AdvanceTurn(IntPtr address);

        [DllImport(sNativeLibraryName)]
        public static extern PlayerColor GetCurrentBoardTurn(IntPtr address);

        [DllImport(sNativeLibraryName)]
        public static extern CastleFlags GetBoardCastlingAvailability(IntPtr address, PlayerColor player);

        [DllImport(sNativeLibraryName)]
        public static extern unsafe bool GetBoardEnPassantTarget(IntPtr address, Coord* target);

        [DllImport(sNativeLibraryName)]
        public static extern ulong GetBoardHalfmoveClock(IntPtr address);

        [DllImport(sNativeLibraryName)]
        public static extern ulong GetBoardFullmoveCount(IntPtr address);

        [DllImport(sNativeLibraryName)]
        public static extern IntPtr GetInternalBoardPointer(IntPtr address);

        #endregion
        #region Engine

        [DllImport(sNativeLibraryName)]
        public static extern IntPtr CreateEngine();

        [DllImport(sNativeLibraryName)]
        public static extern void DestroyEngine(IntPtr address);

        [DllImport(sNativeLibraryName)]
        public static extern IntPtr GetEngineBoard(IntPtr address);

        [DllImport(sNativeLibraryName)]
        public static extern void SetEngineBoard(IntPtr address, IntPtr board);

        public unsafe delegate void EngineCaptureCallback(PieceInfo* piece);
        [DllImport(sNativeLibraryName)]
        public static extern void SetEngineCaptureCallback(IntPtr address, EngineCaptureCallback callback);

        [DllImport(sNativeLibraryName)]
        public static extern IntPtr CreatePieceQuery();
        
        [DllImport(sNativeLibraryName)]
        public static extern void SetQueryPieceType(IntPtr query, PieceType type);

        [DllImport(sNativeLibraryName)]
        public static extern void SetQueryPieceColor(IntPtr query, PlayerColor color);

        [DllImport(sNativeLibraryName)]
        public static extern void SetQueryPieceX(IntPtr query, int x);

        [DllImport(sNativeLibraryName)]
        public static extern void SetQueryPieceY(IntPtr query, int y);

        public unsafe delegate bool QueryFilter(Coord* position, PieceInfo* piece);
        [DllImport(sNativeLibraryName)]
        public static extern void SetQueryFilter(IntPtr query, QueryFilter filter);

        public unsafe delegate void PositionCallback(Coord* position);
        [DllImport(sNativeLibraryName)]
        public static extern void EngineFindPieces(IntPtr address, IntPtr query, PositionCallback callback);

        [DllImport(sNativeLibraryName)]
        public static extern bool EngineComputeCheck(IntPtr address, PlayerColor color, PositionCallback callback);

        [DllImport(sNativeLibraryName)]
        public static extern bool EngineComputeCheckmate(IntPtr address, PlayerColor color);

        [DllImport(sNativeLibraryName)]
        public static extern unsafe void EngineComputeLegalMoves(IntPtr address, Coord* position, PositionCallback callback);

        [DllImport(sNativeLibraryName)]
        public static extern unsafe bool EngineIsMoveLegal(IntPtr address, Move* move);

        [DllImport(sNativeLibraryName)]
        public static extern unsafe bool EngineCommitMove(IntPtr address, Move* move, bool advanceTurn);

        [DllImport(sNativeLibraryName)]
        public static extern void ClearEngineCache(IntPtr address);

        #endregion
        #region Utilities

        [DllImport(sNativeLibraryName)]
        public static extern unsafe IntPtr SerializeCoordinate(Coord* coord);

        [DllImport(sNativeLibraryName)]
        public static extern unsafe bool ParseCoordinate(string src, Coord* result);

        [DllImport(sNativeLibraryName)]
        public static extern void FreeMemory(IntPtr block);

        #endregion
    }
}