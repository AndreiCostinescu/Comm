namespace DesignAIRobotics.Comm.utils {
    class Utils {
        public static int strcmp(string s1, string s2) {
            int i = 0, s1Len = s1.Length, s2Len = s2.Length;
            while (true) {
                if (i == s1Len) {
                    return (i == s2Len) ? 0 : -s2[i];
                }
                if (i == s2Len) {
                    return (i == s1Len) ? 0 : s1[i];
                }
                if (s1[i] != s2[i]) {
                    return s1[i] - s2[i];
                }
                i++;
            }
        }

        public static void memcpy(byte[] destBuffer, ulong destStart, byte[] srcBuffer, ulong srcStart, ulong size) {
            for (ulong i = 0; i < size; i++) {
                destBuffer[destStart + i] = srcBuffer[srcStart + i];
            }
        }

        public static void memset(byte[] destBuffer, ulong destStart, byte value, ulong size) {
            for (ulong i = 0; i < size; i++) {
                destBuffer[destStart + i] = value;
            }
        }


    }
}
