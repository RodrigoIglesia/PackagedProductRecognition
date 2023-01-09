/* Computer vision interface.
 ..
*/

using System;
using System.Windows;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.IO;
using System.IO.Pipes;
using System.Windows.Threading;

namespace VisionInterface
{
    public partial class MainWindow : Window
    {
        int comm_ini = 0;
        public MainWindow()
        {
            InitializeComponent();

            // App executed each sec.
            DispatcherTimer timer = new DispatcherTimer();
            timer.Interval = TimeSpan.FromSeconds(1);
            // Ticker event.
            timer.Tick += timer_Tick;
            timer.Start();
        }

        public ImageSource byteArrayToImage(byte[] byteArrayIn)
        {
            // Convert bytearray to image.
            BitmapImage bmpImage = new BitmapImage();
            MemoryStream ms = new MemoryStream(byteArrayIn);

            bmpImage.BeginInit();
            bmpImage.StreamSource = ms;
            bmpImage.EndInit();

            ImageSource returnImage = bmpImage as ImageSource;

            return returnImage;
        }

        void timer_Tick(object sender, EventArgs e)
        {
            //byte[] buffer = new byte[353172];
            byte[] buffer = new byte[5763072];
            byte[] numBuffer = new byte[4];
            string prediction;
            int numCaja;
            ImageSource image;


            if (comm_ini == 1)
            {
                // Running LED ON.
                processRuning.Opacity = new Double();
                processRuning.Opacity = 1.0;
                processStopped.Opacity = new Double();
                processStopped.Opacity = 0.25;
                // Connect to server PIPE.
                NamedPipeClientStream pipeClient = new NamedPipeClientStream(@".", @"Pipe", PipeDirection.InOut);
                Console.Write("Connecting to Pipe...");
                pipeClient.Connect();

                Console.WriteLine("Coonected!");

                // Server response.
                if (pipeClient.Read(buffer, 0, buffer.Length) > 0)
                {
                    Console.WriteLine("Image received, size: " + buffer.Length.ToString());
                    image = byteArrayToImage(buffer);

                    receivedImage.Source = image;
                }

                if (pipeClient.Read(numBuffer, 0, numBuffer.Length) > 0)
                {
                    if (BitConverter.IsLittleEndian)
                        Array.Reverse(numBuffer);
                    numCaja = BitConverter.ToInt32(numBuffer, 0);
                    Console.WriteLine("Received data: " + numCaja.ToString());
                    numLabel.Content = numCaja.ToString();
                }

                StreamReader sr = new StreamReader(pipeClient);
                prediction = "";
                prediction = sr.ReadLine();
                Console.WriteLine(prediction);
                if (prediction != null)
                {
                    predLabel.Content = prediction;
                }
            }
            else
            {
                processRuning.Opacity = new Double();
                processRuning.Opacity = 0.25;
                processStopped.Opacity = new Double();
                processStopped.Opacity = 1.0;
            }
        }

        private void endButton_Click(object sender, RoutedEventArgs e)
        {
            MessageBoxResult messageBoxResult = System.Windows.MessageBox.Show("Stop vision process?", "Vision Stop.", System.Windows.MessageBoxButton.YesNo);
            if (messageBoxResult == MessageBoxResult.Yes)
                comm_ini = 0;
        }

        private void startButton_Click(object sender, RoutedEventArgs e)
        {
            MessageBoxResult messageBoxResult = System.Windows.MessageBox.Show("Run vision process?", "Vision Run.", System.Windows.MessageBoxButton.YesNo);
            if (messageBoxResult == MessageBoxResult.Yes)
                comm_ini = 1;
        }
    }
}
