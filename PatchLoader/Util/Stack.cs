using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace PatchLoader.Util
{
    /// <summary>
    /// A helper wrapper for a generic stack.
    /// </summary>
    /// <remarks>
    /// Made for use in <see cref="SimpleJSON"/> without the need of importing anything.
    /// </remarks>
    /// <typeparam name="T">Type of the objec to contain.</typeparam>
    public class Stack<T> : System.Collections.Stack where T : class
    {
        public bool Contains(T obj)
        {
            return base.Contains(obj);
        }

        public new T Peek()
        {
            return base.Peek() as T;
        }

        public new T Pop()
        {
            return base.Pop() as T;
        }

        public void Push(T obj)
        {
            base.Push(obj);
        }
    }
}
