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
    [StructLayout(LayoutKind.Sequential)]
    public struct Coord : IEquatable<Coord>
    {
        public int X, Y;

        public Coord()
        {
            X = Y = 0;
        }

        public Coord(int x, int y)
        {
            X = x;
            Y = y;
        }

        public readonly int TaxicabLength => Math.Abs(X) + Math.Abs(Y);

        public override unsafe string ToString()
        {
            fixed (Coord* coord = &this)
            {
                var ptr = NativeFunctions.SerializeCoordinate(coord);
                var result = Marshal.PtrToStringAuto(ptr) ?? throw new ArgumentException();

                NativeFunctions.FreeMemory(ptr);
                return result;
            }
        }

        public static unsafe Coord Parse(string coord)
        {
            var result = new Coord();
            if (!NativeFunctions.ParseCoordinate(coord, &result))
            {
                throw new ArgumentException("Failed to parse coordinate!");
            }

            return result;
        }

        public static unsafe bool TryParse(string coord, out Coord result)
        {
            var tempResult = new Coord();
            bool success = NativeFunctions.ParseCoordinate(coord, &tempResult);

            result = tempResult;
            return success;
        }

        public override bool Equals(object? obj)
        {
            if (obj is Coord coord)
            {
                return Equals(coord);
            }
            else
            {
                return false;
            }
        }

        public bool Equals(Coord coord) => X == coord.X && Y == coord.Y;
        public override int GetHashCode() => (Y << 1) ^ X;

        public static bool operator ==(Coord lhs, Coord rhs) => lhs.Equals(rhs);
        public static bool operator !=(Coord lhs, Coord rhs) => !lhs.Equals(rhs);
        public static implicit operator Coord((int x, int y) t) => new Coord(t.x, t.y);

        public static Coord operator +(Coord lhs, Coord rhs) => (lhs.X + rhs.X, lhs.Y + rhs.Y);
        public static Coord operator +(Coord lhs, int rhs) => lhs + (rhs, rhs);
        public static Coord operator +(int lhs, Coord rhs) => rhs + lhs;

        public static Coord operator -(Coord coord) => (-coord.X, -coord.Y);
        public static Coord operator -(Coord lhs, Coord rhs) => lhs + -rhs;
        public static Coord operator -(Coord lhs, int rhs) => lhs - (rhs, rhs);
        public static Coord operator -(int lhs, Coord rhs) => (lhs, lhs) - rhs;

        public static Coord operator *(Coord lhs, int rhs) => (lhs.X * rhs, lhs.Y * rhs);
        public static Coord operator *(int lhs, Coord rhs) => rhs * lhs;
    }
}