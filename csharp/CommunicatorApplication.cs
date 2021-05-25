using Comm.communication;
using Comm.data;
using Comm.socket;
using Comm.utils;
using System;

namespace Comm {
    public class CommunicatorApplication {
        public CommunicatorApplication() {
            this.quit = false;
            this.state = CommunicatorState.COMMUNICATOR_IDLE;
            this.dataCollection = new DataCollection();
        }

        ~CommunicatorApplication() { }

        public virtual void main() {
            while (!this.quit) {
                this._preMain();
                try {
                    this._main();
                } catch (Exception e) {
                    Console.WriteLine("Exception caught: " + e.ToString());
                }
                this._postMain();
            }
        }

        public virtual void stop() {
            this.quit = true;
        }

        protected bool send(Communication comm, SocketType socketType, CommunicationData data, bool withHeader, int retries = 0, bool verbose = false) {
            return Communicator.send(comm, socketType, data, withHeader, true, retries, verbose);
        }

        protected bool send(Communication comm, SocketType socketType, byte[] data, int dataSize, int retries = 0, bool verbose = false) {
            return Communicator.send(comm, socketType, data, dataSize, retries, verbose);
        }

        protected bool syphon(Communication comm, SocketType socketType, ref MessageType messageType, CommunicationData data, bool withHeader, int retries = 0, bool verbose = false, int syphonRetries = 10) {
            return Communicator.syphon(comm, socketType, ref messageType, data, withHeader, ref this.quit, retries, verbose);
        }

        protected bool listen(Communication comm, SocketType socketType, ref MessageType messageType, ref DataCollection _dataCollection, bool withHeader, int retries = 0, bool verbose = false) {
            return Communicator.listen(comm, socketType, ref messageType, ref _dataCollection, withHeader, ref this.quit, retries, verbose);
        }

        protected bool listenFor(Communication comm, SocketType socketType, CommunicationData data, bool withHeader,
                                 Reference<bool> timeoutResult = null, int countIgnoreOther = -1,
                                 int countOther = -1, int retries = 0, bool verbose = false) {
            return Communicator.listenFor(comm, socketType, data, withHeader, ref this.quit, timeoutResult, countIgnoreOther, countOther, retries, verbose);
        }

        protected virtual void _preMain() { }

        protected virtual void _main() { }

        protected virtual void _postMain() { }

        protected bool quit;
        protected CommunicatorState state;
        protected DataCollection dataCollection;
    };
}
