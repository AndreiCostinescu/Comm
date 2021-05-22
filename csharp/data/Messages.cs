namespace Comm.data {
    public static class Messages {
        public static readonly string ACCEPT_MESSAGE = "Accepted\0";
        public static readonly int ACCEPT_MESSAGE_LENGTH = 9;
        public static readonly string READY_MESSAGE = "Ready\0";
        public static readonly int READY_MESSAGE_LENGTH = 6;
        public static readonly string WAIT_MESSAGE = "Waiting\0";
        public static readonly int WAIT_MESSAGE_LENGTH = 8;
        public static readonly string REJECT_MESSAGE = "Rejected\0";
        public static readonly int REJECT_MESSAGE_LENGTH = 9;
        public static readonly string TOKEN_MESSAGE = "Token\0";
        public static readonly int TOKEN_MESSAGE_LENGTH = 6;
        public static readonly string CALIBRATING_MESSAGE = "Calibrating\0";
        public static readonly int CALIBRATING_MESSAGE_LENGTH = 12;
        public static readonly string CALIBRATION_COMPLETED_MESSAGE = "Calibration completed\0";
        public static readonly int CALIBRATION_COMPLETED_MESSAGE_LENGTH = 21;

        public static readonly string STOP_MESSAGE = "Stop\0";
        public static readonly int STOP_MESSAGE_LENGTH = 5;
        public static readonly string START_MESSAGE = "Start\0";
        public static readonly int START_MESSAGE_LENGTH = 6;
        public static readonly string CONTROL_MESSAGE = "Control\0";
        public static readonly int CONTROL_MESSAGE_LENGTH = 8;
        public static readonly string UPLOAD_MESSAGE = "Upload\0";
        public static readonly int UPLOAD_MESSAGE_LENGTH = 7;
        public static readonly string SELECT_MESSAGE = "Select\0";
        public static readonly int SELECT_MESSAGE_LENGTH = 7;
        public static readonly string PING_MESSAGE = "Ping\0";
        public static readonly int PING_MESSAGE_LENGTH = 5;
        public static readonly string IDLE_MESSAGE = "Idle\0";
        public static readonly int IDLE_MESSAGE_LENGTH = 5;
        public static readonly string ACTIVE_MESSAGE = "Active\0";
        public static readonly int ACTIVE_MESSAGE_LENGTH = 7;
        public static readonly string DONE_MESSAGE = "DONE\0";
        public static readonly int DONE_MESSAGE_LENGTH = 5;
        public static readonly string QUIT_MESSAGE = "Quit\0";
        public static readonly int QUIT_MESSAGE_LENGTH = 5;
    }
}
