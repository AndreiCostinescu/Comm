using System;
using System.Net;

namespace DesignAIRobotics.Comm.socket {
    public class SocketPartner {
        public SocketPartner(bool overwritePartner = false, bool initializePartner = false) {
            this.overwrite = overwritePartner;
            if (initializePartner) {
                this.setPartner("0.0.0.0", 0);
            } else {
                this.partner = null;
            }
        }

        public SocketPartner(string ip, int port, bool overwritePartner) {
            this.overwrite = overwritePartner;
            this.setPartner(ip, port);
        }

        public SocketPartner(IPEndPoint partner, bool overwritePartner = false) {
            this.partner = partner;
            this.overwrite = overwritePartner;
        }

        ~SocketPartner() {
            this.cleanup();
        }

        public SocketPartner copy() {
            SocketPartner copy = new SocketPartner(this.partner, this.overwrite);
            return copy;
        }

        public bool isInitialized() {
            return this.partner != null;
        }

        public void setOverwrite(bool _overwrite) {
            this.overwrite = _overwrite;
        }

        public void setPartner(IPEndPoint _partner) {
            this.partner = _partner;
        }

        public void setPartner(string partnerIP, int partnerPort) {
            try {
                // Establish the remote endpoint for the socket. This example uses port 8080 on the local computer. 
                // For localhost: IPHostEntry ipHost = Dns.GetHostEntry(Dns.GetHostName());  
                // For localhost: IPHostEntry ipHost = Dns.GetHostEntry("localhost");
                if (partnerIP != "") {
                    /*
                    IPAddress ipAddr = Dns.GetHostEntry(partnerIP).AddressList[0];
                    /*/
                    IPAddress ipAddr = Dns.GetHostAddresses(partnerIP)[0];
                    //*/
                    this.partner = new IPEndPoint(ipAddr, partnerPort);
                } else {
                    this.partner = new IPEndPoint(IPAddress.Any, partnerPort);
                }
            } catch (Exception e) {
                Console.WriteLine("Unhandled Exception: " + e.ToString());
            }
        }

        public bool getOverwrite() {
            return this.overwrite;
        }

        public IPEndPoint getPartner() {
            return this.partner;
        }

        public string getPartnerString() {
            return this.getStringAddress();
        }

        public string getIP() {
            return this.partner.Address.ToString();
        }

        public int getPort() {
            return this.partner.Port;
        }

        public string getStringAddress() {
            return (this.partner == null) ? "???" : this.partner.ToString();
        }

        public void cleanup() {
            this.partner = null;
        }


        IPEndPoint partner;
        bool overwrite;
    };

}
