using Comm.communication;
using Comm.data;
using Comm.socket;
using System;
using System.Diagnostics;
using System.IO;
using System.Threading;
using System.Threading.Tasks;

namespace Comm {
    public class Client : CommunicatorApplication {
        public Client(string serverIP = "127.0.0.1", int port = 8348) {
            this.serverIP = serverIP;
            this.port = port;
            this.serverPartner = null;
            this.commQuit = false;
            this.beActive = null;
            this.getter = new CoordinateGetter();
            /*
            this.controlRequestView = null;
            this.controlSelectView = null;
            this.robotView = null;
            this.uploadView = null;
            //*/
        }

        ~Client() {
            if (this.serverPartner != null) {
                this.serverPartner.cleanup();
                this.serverPartner = null;
            }
        }

        public override void main() {
            int i = 0;
            Console.WriteLine("Entering Client::main() loop!");
            while (this.continueCommunicationLoop(0) && i++ < 1) {
                // try to connect to the server
                try {
                    if (this.serverPartner != null) {
                        this.serverPartner.cleanup();
                    }
                    this.serverPartner = new Communication();
                    this.serverPartner.createSocket(SocketType.TCP, new SocketPartner(this.serverIP, this.port, false), 0, 2000, 50);
                    Console.WriteLine("Created server partner; before entering Client::process");
                    this.process();
                } catch (Exception e) {
                    Console.WriteLine("Caught exception: " + e.ToString());
                }
                this.serverPartner.cleanup();
                this.serverPartner = null;
                if (this.quit) {
                    break;
                }
                Console.WriteLine("Waiting for two seconds...");
                Thread.Sleep(2000);  // 2000ms = 2s
            }
            Console.WriteLine("Exiting Client::main() loop!");
        }

        public CoordinateGetter getCoordinateGetter() {
            return this.getter;
        }

        public bool connect(ref string errorMessage) {
            try {
                if (this.serverPartner != null) {
                    this.serverPartner.cleanup();
                }
                this.serverPartner = new Communication();
                Console.WriteLine("Connecting to " + this.serverIP + ":" + this.port);
                this.serverPartner.createSocket(SocketType.TCP, new SocketPartner(this.serverIP, this.port, false), 0, 2000, 100);
                return true;
            } catch (Exception e) {
                Console.WriteLine("Exception in Client.connect: " + e.ToString());
                errorMessage = e.Message;
            }
            return false;
        }

        public bool startCommunication() {
            bool success = false;
            try {
                this.process();
                success = true;
            } catch (System.Net.Sockets.SocketException se) {
                Console.WriteLine("Unhandled socket exception: " + se.ToString());
            } catch (Exception e) {
                Console.WriteLine("Unhandled exception: " + e.ToString());
            }
            /*
            if (this.controlSelectView != null) {
                this.controlSelectView.stopServerCommunication();
            } else if (this.controlRequestView != null) {
                this.controlRequestView = null;
            }
            //*/
            this.serverPartner.cleanup();
            this.serverPartner = null;
            return success;
        }

        private bool continueCommunicationLoop(int level = 0) {
            // Console.WriteLine("Quit: " + this.quit + "; commQuit: " + this.commQuit + "; beActive: " + ((this.beActive == null) ? "???" : this.beActive));
            return (level < 0 ||
                    (!this.quit &&
                     (level < 1 ||
                      (!this.commQuit &&
                       (level < 2 ||
                        (this.beActive == null || Comm.utils.Utils.strcmp(this.beActive, Comm.data.Messages.STOP_MESSAGE) != 0))))));
        }

        private void sendToServerLoop(Communication partner, DataCollection dataCollection) {
            bool sent = true;
            int messageID = 0, verboseMod = 100;

            CoordinateData coordinate = (CoordinateData) dataCollection.get(MessageType.COORDINATE);

            this.getter.resetDrawingArea();
            while (this.continueCommunicationLoop(2) && this.getter.getData(coordinate)) {
                coordinate.set("id", messageID);
                if (messageID % verboseMod == 0) {
                    Console.WriteLine("Send coordinate data: " + coordinate.getID() + ", " + coordinate.getX() + ", " + coordinate.getY() + ", " + coordinate.getTime() + ", " + coordinate.getTouch());
                }
                sent = this.send(partner, SocketType.TCP, coordinate, true);
                if (!sent) {
                    if (partner.getErrorCode() != 0) {
                        Console.WriteLine("Error when sending coordinate data!");
                    }
                    this.commQuit = true;
                    break;
                }
                Thread.Sleep(5);  // sleep for 200Hz period
                messageID++;
            }
            if (!sent) {
                Console.WriteLine("Send problems");
            } else if (this.commQuit) {
                Console.WriteLine("Server sent quit");
            } else if (!this.continueCommunicationLoop(2)) {
                Console.WriteLine("Don't continue...: " + Comm.utils.Utils.strcmp(this.beActive, Comm.data.Messages.STOP_MESSAGE) + "; " + this.beActive);
            } else {
                Console.WriteLine("CoordinateGetter problems!!!");
            }
            this.getter.resetDrawingArea();
        }

        private void recvControlFromServerLoop(Communication partner, DataCollection dataCollection) {
            MessageType messageType;
            int messageID = 0, verboseMod = 10;  // k
            BitmapModel receivedImage;

            ImageData image = (ImageData) dataCollection.get(MessageType.IMAGE);
            StatusData status = (StatusData) dataCollection.get(MessageType.STATUS);

            Console.WriteLine("Before communication loop in recvControlFromServerLoop!");
            Debug.Assert(this.beActive != null);
            while (this.continueCommunicationLoop(2)) {
                messageType = this.processIncomingMessage(partner, SocketType.TCP, ref dataCollection);
                if (!this.continueCommunicationLoop(2)) {
                    Console.WriteLine("Don't continue communication loop...: " + Comm.utils.Utils.strcmp(this.beActive, Comm.data.Messages.STOP_MESSAGE) + "; " + this.beActive);
                    break;
                }
                if (messageType == MessageType.STATUS) {
                    Console.WriteLine("Received from server: \"" + status.getData() + "\"; beActive = " + this.beActive);
                    if (Comm.utils.Utils.strcmp(this.beActive, Comm.data.Messages.STOP_MESSAGE) == 0) {
                        Console.WriteLine("Stopping communicationLoop!");
                        break;
                    }
                } else if (messageType == MessageType.IMAGE) {
                    if (image.isImageDeserialized()) {
                        // Console.WriteLine("Image deserialized correctly!");
                        receivedImage = image.getImage();
                        if (messageID % verboseMod == 0) {
                            Console.WriteLine("Image " + messageID + " is deserialized correctly!\n\t" + receivedImage.height + "x" + receivedImage.width + ": " + receivedImage.type + "; " + receivedImage.opencvBytes + "B");
                        }
                        /*
                        // Console.WriteLine("Previous height: " + this.robotView.recvImage.Height + "; width: " + this.robotView.recvImage.Width);
                        this.robotView.recvImage.MinimumHeightRequest = this.robotView.recvImage.Height;
                        this.robotView.recvImage.MinimumWidthRequest = this.robotView.recvImage.Width;
                        this.robotView.recvImageStream.Value = GenerateImageSource(receivedImage);
                        //*/
                    } else {
                        Console.WriteLine("Error in deserializing image...");
                    }
                    messageID++;
                }
            }
            Console.WriteLine("After communication loop in recvControlFromServerLoop!");
            if (this.commQuit) {
                Console.WriteLine("Server quit");
            } else if (this.beActive != null && Comm.utils.Utils.strcmp(this.beActive, Comm.data.Messages.STOP_MESSAGE) == 0) {
                Console.WriteLine("Server stopped");
            } else {
                Console.WriteLine("User quit!");
            }
        }

        private void recvDataFromServerLoop(Communication partner, DataCollection dataCollection) {
            int messageID = 0, verboseMod = 10;  // k
            MessageType messageType;
            // string windowName = "Received image!";

            ImageData image = (ImageData) dataCollection.get(MessageType.IMAGE);
            StatusData status = (StatusData) dataCollection.get(MessageType.STATUS);

            Console.WriteLine("Before communication loop in recvDataFromServerLoop!");
            while (this.continueCommunicationLoop(2)) {
                // messageType = this.processIncomingMessage(partner, SocketType.UDP, ref dataCollection);
                messageType = this.processIncomingMessage(partner, SocketType.UDP_HEADER, ref dataCollection);
                if (messageType != MessageType.NOTHING) {
                    Console.WriteLine("Received on UDP_HEADER message type: " + MessageTypeConverter.messageTypeToString(messageType));
                }
                if (messageType == MessageType.IMAGE) {
                    if (image.isImageDeserialized()) {
                        // Console.WriteLine("Image deserialized correctly!");
                        BitmapModel receivedImage = image.getImage();
                        if (messageID % verboseMod == 0) {
                            Console.WriteLine("Image " + messageID + " is deserialized correctly!\n\t" + receivedImage.height + "x" + receivedImage.width + ": " + receivedImage.type + "; " + receivedImage.opencvBytes + "B");
                        }
                        /*
                        this.robotView.recvImageStream.Value = GenerateImageSource(receivedImage);
                        //*/
                    } else {
                        Console.WriteLine("Error in deserializing image...");
                    }
                    messageID++;
                }
            }
            Console.WriteLine("After communication loop in recvDataFromServerLoop!");
            if (this.quit) {
                Console.WriteLine("User quit");
                status.setCommand("quit");
                this.send(this.serverPartner, SocketType.TCP, status, true);
            } else if (this.commQuit) {
                Console.WriteLine("Server quit");
            } else if (this.beActive != null && Comm.utils.Utils.strcmp(this.beActive, Comm.data.Messages.STOP_MESSAGE) == 0) {
                Console.WriteLine("Server stopped");
            } else {
                Console.WriteLine("CoordinateGetter problems!!!");
            }
        }

        private void communicationLoop() {
            Console.WriteLine("Entering communication loop!");

            StatusData s = new StatusData();
            s.setCommand("control");
            // s.setCommand("stop");
            if (!this.send(this.serverPartner, SocketType.UDP_HEADER, s, true)) {
                Console.WriteLine("Can not send " + s.getData() + " to udp: " + this.serverPartner.getPartnerString(SocketType.UDP_HEADER) + 
                                  ". error = " + this.serverPartner.getErrorString());
                this.commQuit = true;
                return;
            }

            Debug.Assert(this.quit || this.continueCommunicationLoop(2));

            Communication controlServerPartner = this.serverPartner.copy();
            Communication dataServerPartner = this.serverPartner.copy();
            DataCollection dataCollectionCopy = new DataCollection();

            // Task dataThread = Task.Run(() => { this.recvDataFromServerLoop(dataServerPartner, dataCollectionCopy); });
            Task controlThread = Task.Run(() => { this.recvControlFromServerLoop(controlServerPartner, dataCollectionCopy); });

            this.sendToServerLoop(this.serverPartner, this.dataCollection);
            Debug.Assert(!this.continueCommunicationLoop(2));

            // dataThread.Wait();
            controlThread.Wait();

            Debug.Assert(!this.continueCommunicationLoop(2));
            controlServerPartner.cleanup();
            dataServerPartner.cleanup();
            Console.WriteLine("Exiting communication loop!");
        }

        private void uploadLoop() {
            ;
        }

        private void process() {
            StatusData status = (StatusData) this.dataCollection.get(MessageType.STATUS);
            this.state = CommunicatorState.COMMUNICATOR_IDLE;
            string statusResult;

            this.commQuit = false;
            while (this.continueCommunicationLoop(1)) {
                // loop for ping messages
                while (this.continueCommunicationLoop(1)) {
                    this.processIncomingMessage(this.serverPartner, SocketType.TCP, ref this.dataCollection);
                    statusResult = status.getData();
                    if (statusResult != null) {
                        Console.WriteLine("Received from server: " + statusResult);
                        if (Comm.utils.Utils.strcmp(statusResult, Comm.data.Messages.START_MESSAGE) == 0) {
                            break;
                        }
                    }
                }

                /*
                // switch to control select view
                Console.WriteLine("Starting control select view!");
                try {
                    this.controlRequestView.startServerCommunication();
                } catch (Exception e) {
                    Console.WriteLine("Caught exception when changing to new page: " + e.ToString());
                    this.controlRequestView.debugOutput.Value = e.Message;
                    this.quit = true;
                    break;
                }
                while (this.controlSelectView == null) { }
                //*/
                this.state = CommunicatorState.COMMUNICATOR_ACTIVE;

                // recv the udp port 
                MessageType messageType = MessageType.NOTHING;
                while (this.continueCommunicationLoop(1)) {
                    if (!this.listen(this.serverPartner, SocketType.TCP, ref messageType, ref this.dataCollection, true, 10)) {
                        Console.WriteLine("Can not receive data from " + this.serverPartner.getPartnerString(SocketType.TCP) + 
                                          "; error " + this.serverPartner.getErrorString());
                        Console.WriteLine("Can not receive the expected udp port from " + this.serverPartner.getPartnerString(SocketType.TCP) + 
                                          ". error = " + this.serverPartner.getErrorString());
                        this.commQuit = true;
                        return;
                    }
                    if (messageType == MessageType.NOTHING) {
                        continue;
                    } else if (messageType != MessageType.STATUS) {
                        Console.WriteLine("Error in communication protocol with " + this.serverPartner.getPartnerString(SocketType.TCP) + 
                                          ". Expected status data containing udp port not " + MessageTypeConverter.messageTypeToString(messageType));
                        this.commQuit = true;
                        return;
                    }
                    break;
                }
                if (!this.continueCommunicationLoop(1)) {
                    Console.WriteLine("Stopped because don't continue communication loop!");
                    break;
                }

                int serverUDPPort;
                try {
                    serverUDPPort = Int32.Parse(status.getData());
                } catch (Exception e) {
                    Console.WriteLine("Caught exception when trying to convert the received string udp port to int: " + e.ToString());
                    this.commQuit = true;
                    return;
                }
                Console.WriteLine("Received the udp port to communicate on: " + serverUDPPort);
                this.serverPartner.createSocket(SocketType.UDP_HEADER, new SocketPartner(this.serverIP, serverUDPPort, false), 0, 2000, 50);

                // second loop for ping messages until control mode has been selected
                string controlMode = "";
                while (this.continueCommunicationLoop(1)) {
                    this.processIncomingMessage(this.serverPartner, SocketType.TCP, ref this.dataCollection);
                    statusResult = status.getData();
                    if (statusResult != null) {
                        Console.WriteLine("Received from server: " + statusResult);
                        if (Comm.utils.Utils.strcmp(statusResult, Comm.data.Messages.STOP_MESSAGE) == 0) {
                            break;
                        }
                    }
                    controlMode = "control";
                    break;
                    /*
                    if (this.robotView != null) {
                        controlMode = "control";
                        break;
                    }
                    if (this.uploadView != null) {
                        controlMode = "upload";
                        break;
                    } 
                    //*/
                }
                if (controlMode == "") {
                    continue;
                } else if (controlMode == "control") {
                    this.communicationLoop();
                } else if (controlMode == "upload") {
                    this.uploadLoop();
                }

                this.state = CommunicatorState.COMMUNICATOR_IDLE;
            }

            /*
            if (this.continueCommunicationLoop(0)) {
                try {
                    this.controlSelectView.stopServerCommunication();
                } catch (Exception e) {
                    Console.WriteLine("Caught exception when changing to new page: " + e.ToString());
                    // this.controlRequestView.debugOutput.Value = e.Message;
                    Console.WriteLine("STOPPED SERVER COMMUNICATION!");
                }
            }
            //*/
            this.state = CommunicatorState.COMMUNICATOR_DONE;
        }

        private MessageType processIncomingMessage(Communication partner, SocketType type, ref DataCollection dataCollection) {
            MessageType messageType = MessageType.NOTHING;
            // cout << "Listening for data..." << endl;
            StatusData status = (StatusData) dataCollection.get(MessageType.STATUS);
            if (!this.listen(partner, type, ref messageType, ref dataCollection, true)) {
                status.reset();
                return messageType;
            }

            if (messageType != MessageType.NOTHING && messageType != MessageType.STATUS && messageType != MessageType.IMAGE) {
                throw new Exception("Can only process nothing, status or image data.Messages...");
            }
            if (messageType == MessageType.NOTHING || messageType == MessageType.IMAGE) {
                status.reset();
                return messageType;
            }

            Console.WriteLine("Received from server: " + status.getData() + " " + MessageTypeConverter.messageTypeToString(messageType));

            string listenResult = status.getData();
            if (Comm.utils.Utils.strcmp(listenResult, Comm.data.Messages.PING_MESSAGE) == 0) {
                status.setStatus(CommunicatorStateConverter.convertCommunicatorStateToStatus(this.state));
                Console.WriteLine("Sending to server " + status.getData());
                if (!this.send(partner, SocketType.TCP, status, true)) {
                    Console.WriteLine("Can not send ping response to " + partner.getPartnerString(SocketType.TCP));
                } else {
                    Console.WriteLine("Sent!");
                }
            } else if (Comm.utils.Utils.strcmp(listenResult, Comm.data.Messages.QUIT_MESSAGE) == 0) {
                this.commQuit = true;
            } else if (Comm.utils.Utils.strcmp(listenResult, Comm.data.Messages.START_MESSAGE) == 0 || Comm.utils.Utils.strcmp(listenResult, Comm.data.Messages.STOP_MESSAGE) == 0) {
                this.beActive = listenResult;
                return messageType;
            } else if (Comm.utils.Utils.strcmp(listenResult, Comm.data.Messages.WAIT_MESSAGE) == 0) {
            } else {
                Console.WriteLine("Unknown status command " + listenResult);
                this.commQuit = true;
            }
            status.reset();
            return MessageType.NOTHING;
        }

        private string serverIP;
        private string beActive;
        private int port;
        private bool commQuit;
        private CoordinateGetter getter;
        private Communication serverPartner;

        /*
        public void setControlRequestView(ControlRequestView controlRequestView) {
            this.controlRequestView = controlRequestView;
        }

        public void setControlSelectView(ControlSelectView controlSelectView) {
            this.controlSelectView = controlSelectView;
        }

        public void setControlRobotView(ControlRobotView controlRobotView) {
            this.robotView = controlRobotView;
        }

        public void setControlUploadView(ControlUploadView controlUploadView) {
            this.uploadView = controlUploadView;
        }

        private static StreamImageSource GenerateImageSource(BitmapModel image) {
            // Create MemoryStream from buffer with bitmap; Convert to StreamImageSource.
            return (StreamImageSource) StreamImageSource.FromStream(() => {
                return new MemoryStream(image.buffer);
            });
        }

        private ControlRequestView controlRequestView;
        private ControlSelectView controlSelectView;
        private ControlRobotView robotView;
        private ControlUploadView uploadView;
        //*/
    };
}
