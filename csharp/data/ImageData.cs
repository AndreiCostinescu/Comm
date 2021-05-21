using System;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Text;

namespace DesignAIRobotics.Comm.data {
    public enum BitmapFormat {
        Empty = 0,
        Grayscale = 1,
        RGB = 3,
        RGBA = 4,
    }

    public static class BitmapFormatConverter {
        public static BitmapFormat ConvertFromInt(int format) {
            switch (format) {
                case 1: {
                    return BitmapFormat.Grayscale;
                }
                case 3: {
                    return BitmapFormat.RGB;
                }
                case 4: {
                    return BitmapFormat.RGBA;
                }
                default: {
                    return BitmapFormat.Empty;
                }
            }
        }

        public static int ConvertToOpenCV(BitmapFormat format) {
            switch (format) {
                case BitmapFormat.Grayscale: {
                    return 0;
                }
                case BitmapFormat.RGB: {
                    return 16;
                }
                case BitmapFormat.RGBA: {
                    return 24;
                }
                default: {
                    return -1;
                }
            }
        }

        public static BitmapFormat ConvertFromOpenCV(int format) {
            switch ((format / 8) % 8) {
                case 0: {
                    return BitmapFormat.Grayscale;
                }
                case 2: {
                    return BitmapFormat.RGB;
                }
                case 3: {
                    return BitmapFormat.RGBA;
                }
                default: {
                    return BitmapFormat.Empty;
                }
            }
        }
    }

    public class BitmapColor {
        public byte r, g, b, a;

        public BitmapColor(byte r, byte g, byte b, byte a) {
            this.r = r;
            this.g = g;
            this.b = b;
            this.a = a;
        }
    };

    public class BitmapModel {
        public const int headerSize = 54;
        public byte[] buffer;
        public int numPixels, bmpPixelBytes, width, height, type, opencvBytes;
        public BitmapFormat format;
        private bool initializedBitmapHeader;

        public BitmapModel() {
            this.buffer = null;
            this.width = 0;
            this.height = 0;
            this.numPixels = 0;
            this.bmpPixelBytes = 0;
            this.opencvBytes = 0;
            this.format = BitmapFormat.Empty;
            this.initializedBitmapHeader = false;
        }

        public BitmapModel(int height, int width, BitmapFormat format) {
            this.UpdateBitmapHeader(width, height, BitmapFormatConverter.ConvertToOpenCV(format));
        }

        public BitmapModel(int height, int width, int format) {
            this.UpdateBitmapHeader(width, height, format);
        }

        private int initHeaderData(int width, int height, int type) {
            this.width = width;
            this.height = height;
            this.type = type;
            this.format = BitmapFormatConverter.ConvertFromOpenCV(type);
            this.numPixels = this.width * this.height;
            this.bmpPixelBytes = 4 * this.numPixels;

            int factor = 0;
            if (this.format == BitmapFormat.Grayscale) {
                factor = 1;
            } else if (this.format == BitmapFormat.RGB) {
                factor = 3;
            } else if (this.format == BitmapFormat.RGBA) {
                factor = 4;
            }
            this.opencvBytes = factor * this.numPixels;

            int fileSize = headerSize + this.bmpPixelBytes;
            this.buffer = new byte[fileSize];

            return fileSize;
        }

        private void writeHeaderData(int fileSize) {
            // Write headers in MemoryStream and hence the buffer.
            using (MemoryStream memoryStream = new MemoryStream(this.buffer)) {
                using (BinaryWriter writer = new BinaryWriter(memoryStream, Encoding.UTF8)) {
                    if (this.initializedBitmapHeader) {
                        writer.Seek(2, SeekOrigin.Begin);    // jump 2 bytes from origin to fileSize
                        writer.Write(fileSize);              // File size
                        writer.Seek(44, SeekOrigin.Begin);   // jump 44 bytes from origin to width
                        writer.Write(this.width);            // Pixel width
                        writer.Write(this.height);           // Pixel height
                        writer.Seek(8, SeekOrigin.Current);  // jump 8 bytes from here to numPixelBytes
                        writer.Write(this.bmpPixelBytes);    // Image size in bytes
                    } else {
                        // Construct BMP header (14 bytes).
                        writer.Write(new char[] { 'B', 'M' });  // 2: Signature
                        writer.Write(fileSize);                 // 4: File size
                        writer.Write((short) 0);                 // 2: Reserved
                        writer.Write((short) 0);                 // 2: Reserved
                        writer.Write(headerSize);               // 4: Offset to pixels

                        // Construct BitmapInfoHeader (40 bytes).
                        writer.Write(40);                       // 4: Header size
                        writer.Write(this.width);               // 4: Pixel width
                        writer.Write(this.height);              // 4: Pixel height
                        writer.Write((short) 1);                 // 2: Planes
                        writer.Write((short) 32);                // 2: Bits per pixel
                        writer.Write(0);                        // 4: Compression
                        writer.Write(this.bmpPixelBytes);       // 4: Image size in bytes
                        writer.Write(0);                        // 4: X pixels per meter
                        writer.Write(0);                        // 4: Y pixels per meter
                        writer.Write(0);                        // 4: Number colors in color table
                        writer.Write(0);                        // 4: Important color count

                        this.initializedBitmapHeader = true;
                    }
                }
            }
        }

        public void UpdateBitmapHeader(int width, int height, BitmapFormat format) {
            this.UpdateBitmapHeader(width, height, BitmapFormatConverter.ConvertToOpenCV(format));
        }

        public void UpdateBitmapHeader(int width, int height, int type) {
            if (this.width == width && this.height == height && this.type == type) {
                return;
            }

            this.writeHeaderData(this.initHeaderData(width, height, type));
        }

        public void SetData(int width, int height, int type, byte[] data) {
            this.SetData(width, height, BitmapFormatConverter.ConvertFromOpenCV(type), data);
        }

        public void SetData(byte[] data) {
            Debug.Assert(data.Length == this.opencvBytes);

            switch (format) {
                case BitmapFormat.Grayscale: {
                    for (int i = 0; i < this.numPixels; i++) {
                        this.buffer[headerSize + 4 * i + 0] = data[i];
                        this.buffer[headerSize + 4 * i + 1] = data[i];
                        this.buffer[headerSize + 4 * i + 2] = data[i];
                        this.buffer[headerSize + 4 * i + 3] = 255;
                    }
                    break;
                }
                case BitmapFormat.RGB: {
                    for (int i = 0; i < this.numPixels; i++) {
                        this.buffer[headerSize + 4 * i + 0] = data[3 * i];
                        this.buffer[headerSize + 4 * i + 1] = data[3 * i + 1];
                        this.buffer[headerSize + 4 * i + 2] = data[3 * i + 2];
                        this.buffer[headerSize + 4 * i + 3] = 255;
                    }
                    break;
                }
                case BitmapFormat.RGBA: {
                    utils.Utils.memcpy(this.buffer, headerSize, data, 0, (ulong) this.opencvBytes);
                    // Array.Copy(data, 0, this.buffer, headerSize, this.numPixelBytes);
                    break;
                }
                default: {
                    break;
                }
            }
        }

        public void SetData(int width, int height, BitmapFormat format, byte[] data) {
            this.UpdateBitmapHeader(width, height, format);
            this.SetData(data);
        }

        public byte[] GetRGBData() {
            byte[] resBuffer = new byte[this.numPixels * 3];
            for (int i = 0; i < this.numPixels; i++) {
                resBuffer[3 * i + 0] = this.buffer[headerSize + 4 * i + 0];
                resBuffer[3 * i + 1] = this.buffer[headerSize + 4 * i + 1];
                resBuffer[3 * i + 2] = this.buffer[headerSize + 4 * i + 2];
            }
            return resBuffer;
        }

        public void Fill(int x, bool opencvStyle = false) {
            this.Fill(x, x, x, opencvStyle);
        }

        public void Fill(int r, int g, int b, bool opencvStyle = false) {
            this.Fill(r, g, b, 255, opencvStyle);
        }

        public void Fill(int r, int g, int b, int a, bool opencvStyle = false) {
            int index = headerSize;
            for (int i = 0; i < this.numPixels; i++, index += 4) {
                if (opencvStyle) {
                    this.buffer[index + 0] = (byte) b;
                    this.buffer[index + 1] = (byte) g;
                    this.buffer[index + 2] = (byte) r;
                } else {
                    this.buffer[index + 0] = (byte) r;
                    this.buffer[index + 1] = (byte) g;
                    this.buffer[index + 2] = (byte) b;
                }
                this.buffer[index + 3] = (byte) a;
            }
        }

        public void SetPixel(int row, int col, Color color, bool opencvStyle = false) {
            this.SetPixel(row, col, (int) (255 * color.R), (int) (255 * color.G), (int) (255 * color.B), (int) (255 * color.A), opencvStyle);
        }

        public void SetPixel(int row, int col, BitmapColor color, bool opencvStyle = false) {
            this.SetPixel(row, col, color.r, color.g, color.b, color.a, opencvStyle);
        }

        public void SetPixel(int row, int col, int r, int g, int b, int a = 255, bool opencvStyle = false) {
            int index = (row * this.width + col) * 4 + headerSize;
            if (opencvStyle) {
                this.buffer[index + 0] = (byte) b;
                this.buffer[index + 1] = (byte) g;
                this.buffer[index + 2] = (byte) r;
            } else {
                this.buffer[index + 0] = (byte) r;
                this.buffer[index + 1] = (byte) g;
                this.buffer[index + 2] = (byte) b;
            }
            this.buffer[index + 3] = (byte) a;
        }

        public BitmapColor GetPixel(int row, int col) {
            int index = (row * this.width + col) * 4 + headerSize;
            return new BitmapColor(this.buffer[index], this.buffer[index + 1], this.buffer[index + 2], this.buffer[index + 3]);
        }

        public int GetOpenCVType() {
            return this.type;
        }

        // For Xamarin
        /*
        public ImageSource Generate() {
            // Create MemoryStream from buffer with bitmap; Convert to StreamImageSource.
            return ImageSource.FromStream(() => {
                return new MemoryStream(this.buffer);
            });
        }
        //*/
    }

    public class ImageData : CommunicationData {
        public static readonly ulong headerSize = 5 * sizeof(int);

        public ImageData() {
            this.image = new BitmapModel();
            this.id = -1;
            this.imageHeight = 0;
            this.imageWidth = 0;
            this.imageType = 0;
            this.contentSize = 0;
            this.imageDeserialized = true;
        }

        public ImageData(BitmapModel image, int id) {
            this.setID(id);
            this.setImage(image, true);
        }

        public override MessageType getMessageType() {
            return MessageType.IMAGE;
        }

        public override bool serialize(utils.Buffer buffer, int start, bool forceCopy, bool verbose) {
            switch (this.serializeState) {
                case 0: {
                    buffer.setBufferContentSize(headerSize);
                    if (verbose) {
                        Console.WriteLine("Serialize: " + this.imageHeight + ", " + this.imageWidth + ", " + this.imageType + ", " + this.contentSize);
                    }
                    buffer.setInt(this.id, start);
                    buffer.setInt(this.imageHeight, start + 4);
                    buffer.setInt(this.imageWidth, start + 8);
                    buffer.setInt(this.imageType, start + 12);
                    buffer.setInt(this.contentSize, start + 16);
                    if (verbose) {
                        byte[] dataBuffer = buffer.getBuffer();
                        Console.WriteLine("Serialized content: ");
                        for (int i = 0; (ulong) i < headerSize; i++) {
                            Console.WriteLine(((int) dataBuffer[i]) + " ");
                        }
                        Console.WriteLine();
                    }
                    this.serializeState = 1;
                    return false;
                }
                case 1: {
                    if (forceCopy) {
                        buffer.setData(this.image.GetRGBData(), (ulong) this.contentSize, (ulong) start);
                    } else {
                        if (start != 0) {
                            throw new System.Exception("Can not set a reference to data not starting at the first position!");
                        }
                        buffer.setReferenceToData(this.image.GetRGBData(), (ulong) this.contentSize);
                    }
                    this.serializeState = 0;
                    return true;
                }
                default: {
                    Console.WriteLine("Impossible serialize state... " + this.serializeState);
                    this.resetSerializeState();
                    return false;
                }
            }
        }

        public override ulong getExpectedDataSize() {
            switch (this.deserializeState) {
                case 0: {
                    return headerSize;
                }
                case 1: {
                    return (ulong) this.contentSize;
                }
                default: {
                    throw new Exception("Impossible deserialize state... " + this.deserializeState);
                }
            }
        }

        public override bool deserialize(utils.Buffer buffer, int start, bool forceCopy, bool verbose) {
            switch (this.deserializeState) {
                case 0: {
                    this.imageDeserialized = false;
                    this.id = buffer.getInt(start);
                    this.imageHeight = buffer.getInt(start + 4);
                    this.imageWidth = buffer.getInt(start + 8);
                    this.imageType = buffer.getInt(start + 12);
                    this.contentSize = buffer.getInt(start + 16);
                    this.deserializeState = 1;
                    return false;
                }
                case 1: {
                    this.image = null;
                    this.image = new BitmapModel(this.imageHeight, this.imageWidth, BitmapFormat.RGB);
                    Debug.Assert((ulong) this.contentSize == buffer.getBufferContentSize());
                    this.image.SetData(buffer.getBuffer());

                    this.imageDeserialized = true;
                    this.deserializeState = 0;
                    return true;
                }
                default: {
                    Console.WriteLine("Impossible deserialize state... " + this.deserializeState);
                    this.resetDeserializeState();
                    return false;
                }
            }
        }

        public void setID(int id) {
            this.id = id;
        }

        public void setImage(BitmapModel image, bool withSettingData = true) {
            this.image = image;
            if (withSettingData) {
                this.imageHeight = this.image.height;
                this.imageWidth = this.image.width;
                this.imageType = this.image.GetOpenCVType();
                this.contentSize = this.image.opencvBytes;
                this.imageDeserialized = true;
            }
        }

        public BitmapModel getImage() {
            return this.image;
        }

        public int getID() {
            return this.id;
        }

        public int getHeight() {
            return this.imageHeight;
        }

        public int getWidth() {
            return this.imageWidth;
        }

        public int getType() {
            return this.imageType;
        }

        public byte[] getImageBytes() {
            return this.image.GetRGBData();
        }

        public int getImageBytesSize() {
            return this.contentSize;
        }

        public bool isImageDeserialized() {
            return this.imageDeserialized;
        }


        private BitmapModel image;
        private int id, imageHeight, imageWidth, imageType, contentSize;
        private bool imageDeserialized;
    };
}
