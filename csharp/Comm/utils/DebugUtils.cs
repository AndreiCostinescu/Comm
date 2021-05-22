using System;

namespace Comm.utils {
    public class DebugUtils {
        private static DebugUtils defaultDebugUtilsImplementation = new DebugUtils();
        private static DebugUtils debugUtilsImplementation;

        public virtual void SetOutputImpl(string message) {
            Console.WriteLine(message);
        }

        public static void SetDebugImplementation(DebugUtils debugUtilsImplementation) {
            DebugUtils.debugUtilsImplementation = debugUtilsImplementation;
        }

        public static void ResetDebugImplementation() {
            DebugUtils.debugUtilsImplementation = null;
        }

        public static void SetOutput(string message) {
            if (DebugUtils.debugUtilsImplementation == null) {
                DebugUtils.defaultDebugUtilsImplementation.SetOutputImpl(message);
            } else {
                DebugUtils.debugUtilsImplementation.SetOutputImpl(message);
            }
        }
    }
}
