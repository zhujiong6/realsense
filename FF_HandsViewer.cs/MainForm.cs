using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace hands_viewer.cs
{
    public partial class MainForm : Form
    {
        public PXCMSession g_session;
        private volatile bool closing = false;
        public volatile bool stop = false;
        private static Bitmap bitmap = null;



        private Timer timer = new Timer();
        private string filename = null;
        public Dictionary<string, PXCMCapture.DeviceInfo> Devices { get; set; }

        private string gestureName = null;
        private bool _isInitGesturesFirstTime = false;

        private float pSize = 3.0f;

        private class Item
        {
            public string Name;
            public int Value;
            public Item(string name, int value)
            {
                Name = name; Value = value;
            }
            public override string ToString()
            {
                // Generates the text shown in the combo box
                return Name;
            }
        }


        public MainForm(PXCMSession session)
        {
            InitializeComponent();

            this.g_session = session;
            PopulateDeviceMenu();
            cmbGesturesList.Enabled = false;
            FormClosing += new FormClosingEventHandler(MainForm_FormClosing);
            Panel2.Paint += new PaintEventHandler(Panel_Paint);
            timer.Tick += new EventHandler(timer_Tick);
            timer.Interval = 2000;
            timer.Start();


        }



        private delegate void UpdateGesturesToListDelegate(string gestureName, int index);
        public void UpdateGesturesToList(string gestureName, int index)
        {
            cmbGesturesList.Invoke(new UpdateGesturesToListDelegate(delegate (string name, int cmbIndex) { cmbGesturesList.Items.Add(new Item(name, cmbIndex)); }), new object[] { gestureName, index });
        }

        public void setInitGesturesFirstTime(bool isInit)
        {
            _isInitGesturesFirstTime = isInit;

        }

        private delegate void ResetGesturesListDelegate();
        public void resetGesturesList()
        {
            cmbGesturesList.Invoke(new ResetGesturesListDelegate(delegate () { cmbGesturesList.Text = ""; cmbGesturesList.Items.Clear(); cmbGesturesList.SelectedIndex = -1; cmbGesturesList.Enabled = false; cmbGesturesList.Size = new System.Drawing.Size(100, 20); }), new object[] { });
        }

        private delegate void UpdateGesturesListSizeDelegate();
        public void UpdateGesturesListSize()
        {
            cmbGesturesList.Invoke(new UpdateGesturesListSizeDelegate(delegate () { cmbGesturesList.Enabled = true; cmbGesturesList.Size = new System.Drawing.Size(100, 70); }), new object[] { });

        }

        public bool getInitGesturesFirstTime()
        {
            return _isInitGesturesFirstTime;
        }



        public string GetCheckedSmoother()
        {
            foreach (ToolStripMenuItem m in MainMenu.Items)
            {
                if (!m.Text.Equals("Smoother")) continue;
                foreach (ToolStripMenuItem e in m.DropDownItems)
                {
                    if (e.Checked) return e.Text;
                }
            }
            return null;
        }

        public PXCMCapture.DeviceInfo GetDeviceFromFileMenu(string fileName)
        {
            PXCMSession.ImplDesc desc = new PXCMSession.ImplDesc();
            desc.group = PXCMSession.ImplGroup.IMPL_GROUP_SENSOR;
            desc.subgroup = PXCMSession.ImplSubgroup.IMPL_SUBGROUP_VIDEO_CAPTURE;

            PXCMSession.ImplDesc desc1;
            PXCMCapture.DeviceInfo dinfo;
            PXCMSenseManager pp = PXCMSenseManager.CreateInstance();
            if (pp == null)
            {
                return null;
            }
            if (pp.captureManager == null)
            {
                return null;
            }
            try
            {
                if (g_session.QueryImpl(desc, 0, out desc1) < pxcmStatus.PXCM_STATUS_NO_ERROR) throw null;
                if (pp.captureManager.SetFileName(fileName, false) < pxcmStatus.PXCM_STATUS_NO_ERROR) throw null;
                if (pp.captureManager.LocateStreams() < pxcmStatus.PXCM_STATUS_NO_ERROR) throw null;
                pp.captureManager.device.QueryDeviceInfo(out dinfo);
            }
            catch
            {
                pp.Dispose();
                return null;
            }


            pp.Close();
            pp.Dispose();

            StatusLabel.Text = "Ok";
            return dinfo;
        }

        private void PopulateDeviceMenu()
        {
            Devices = new Dictionary<string, PXCMCapture.DeviceInfo>();

            PXCMSession.ImplDesc desc = new PXCMSession.ImplDesc();
            desc.group = PXCMSession.ImplGroup.IMPL_GROUP_SENSOR;
            desc.subgroup = PXCMSession.ImplSubgroup.IMPL_SUBGROUP_VIDEO_CAPTURE;
            ToolStripMenuItem sm = new ToolStripMenuItem("Device");
            for (int i = 0; ; i++)
            {
                PXCMSession.ImplDesc desc1;
                if (g_session.QueryImpl(desc, i, out desc1) < pxcmStatus.PXCM_STATUS_NO_ERROR) break;
                PXCMCapture capture;
                if (g_session.CreateImpl<PXCMCapture>(desc1, out capture) < pxcmStatus.PXCM_STATUS_NO_ERROR) continue;
                for (int j = 0; ; j++)
                {
                    PXCMCapture.DeviceInfo dinfo;
                    if (capture.QueryDeviceInfo(j, out dinfo) < pxcmStatus.PXCM_STATUS_NO_ERROR) break;
                    string name = dinfo.name;
                    if (Devices.ContainsKey(dinfo.name))
                    {
                        name += j;
                    }
                    Devices.Add(name, dinfo);
                    ToolStripMenuItem sm1 = new ToolStripMenuItem(dinfo.name, null, new EventHandler(Device_Item_Click));
                    sm.DropDownItems.Add(sm1);
                }
                capture.Dispose();
            }
            if (sm.DropDownItems.Count > 0)
                (sm.DropDownItems[0] as ToolStripMenuItem).Checked = true;
            MainMenu.Items.RemoveAt(0);
            MainMenu.Items.Insert(0, sm);
        }




        private void RadioCheck(object sender, string name)
        {
            foreach (ToolStripMenuItem m in MainMenu.Items)
            {
                if (!m.Text.Equals(name)) continue;
                foreach (ToolStripMenuItem e1 in m.DropDownItems)
                {
                    e1.Checked = (sender == e1);
                }
            }
        }

        private void Device_Item_Click(object sender, EventArgs e)
        {
            RadioCheck(sender, "Device");
        }

        private void Module_Item_Click(object sender, EventArgs e)
        {
            RadioCheck(sender, "Module");
        }





        private void Start_Click(object sender, EventArgs e)
        {
            MainMenu.Enabled = false;
            Start.Enabled = false;
            Stop.Enabled = true;

            EnableTrackingMode(false);

            stop = false;
            System.Threading.Thread thread = new System.Threading.Thread(DoRecognition);
            thread.Start();
            System.Threading.Thread.Sleep(5);
        }

        delegate void DoRecognitionCompleted();
        private void DoRecognition()
        {
            HandsRecognition gr = new HandsRecognition(this);
            gr.SimplePipeline();
            this.Invoke(new DoRecognitionCompleted(
                delegate
                {
                    Start.Enabled = true;
                    Stop.Enabled = false;
                    EnableTrackingMode(true);
                    MainMenu.Enabled = true;
                    if (closing) Close();
                }
            ));
        }

        public string GetCheckedDevice()
        {
            foreach (ToolStripMenuItem m in MainMenu.Items)
            {
                if (!m.Text.Equals("Device")) continue;
                foreach (ToolStripMenuItem e in m.DropDownItems)
                {
                    if (e.Checked) return e.Text;
                }
            }
            return null;
        }



        public bool GetDepthState()
        {
            return Depth.Checked;
        }

        public bool GetLabelmapState()
        {
            return Labelmap.Checked;
        }

        public bool GetJointsState()
        {
            return Joints.Checked;
        }

        public bool GetSkeletonState()
        {
            return Skeleton.Checked;
        }

        public bool GetCursorState()
        {
            return Cursor.Checked;
        }

        public bool GetAdaptiveState()
        {
            return Adaptive.Checked;
        }

        public bool GetExtremitiesState()
        {
            return Extremities.Checked;
        }

        public bool GetCursorModeState()
        {
            return CursorMode.Checked;
        }

        public bool GetFullHandModeState()
        {
            return FullHandMode.Checked;
        }

        private void MainForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            stop = true;
            e.Cancel = Stop.Enabled;
            closing = true;
        }

        private delegate void UpdateStatusDelegate(string status);
        public void UpdateStatus(string status)
        {
            Status2.Invoke(new UpdateStatusDelegate(delegate (string s) { StatusLabel.Text = s; }), new object[] { status });
        }

        private delegate void UpdateInfoDelegate(string status, Color color);
        public void UpdateInfo(string status, Color color)
        {
            infoTextBox.Invoke(new UpdateInfoDelegate(delegate (string s, Color c)
            {
                if (status == String.Empty)
                {
                    infoTextBox.Text = String.Empty;
                    return;
                }

                if (infoTextBox.TextLength > 1200)
                {
                    infoTextBox.Text = String.Empty;
                }

                infoTextBox.SelectionColor = c;

                infoTextBox.SelectedText = s;
                infoTextBox.SelectionColor = infoTextBox.ForeColor;

                infoTextBox.SelectionStart = infoTextBox.Text.Length;
                infoTextBox.ScrollToCaret();

            }), new object[] { status, color });
        }



        private delegate void UpdateFPSStatusDelegate(string status);
        public void UpdateFPSStatus(string status)
        {
            labelFPS.Invoke(new UpdateFPSStatusDelegate(delegate (string s) { labelFPS.Text = s; }), new object[] { status });
        }

        private void Stop_Click(object sender, EventArgs e)
        {
            stop = true;
            EnableTrackingMode(true);
            resetGesturesList();
        }

        private delegate void UpdateEnableTrackingModeDelegate(bool flag);
        public void EnableTrackingMode(bool flag)
        {
            CursorMode.Invoke(new UpdateEnableTrackingModeDelegate(delegate (bool enable) { CursorMode.Enabled = enable; }), new object[] { flag });
            //ExtremitiesMode.Invoke(new UpdateEnableTrackingModeDelegate(delegate(bool enable) { ExtremitiesMode.Enabled = enable; }), new object[] { flag });
            FullHandMode.Invoke(new UpdateEnableTrackingModeDelegate(delegate (bool enable) { FullHandMode.Enabled = enable; }), new object[] { flag });
        }

        public void DisplayBitmap(Bitmap picture)
        {
            lock (this)
            {
                if (bitmap != null)
                    bitmap.Dispose();
                bitmap = new Bitmap(picture);
            }
        }

        private void Panel_Paint(object sender, PaintEventArgs e)
        {
            lock (this)
            {
                if (bitmap == null || bitmap.Width == 0 || bitmap.Height == 0) return;
                Bitmap bitmapNew = new Bitmap(bitmap);
                try
                {
                    if (Mirror.Checked)
                    {
                        bitmapNew.RotateFlip(RotateFlipType.RotateNoneFlipX);
                    }

                    if (Scale2.Checked)
                    {
                        /* Keep the aspect ratio */
                        Rectangle rc = (sender as PictureBox).ClientRectangle;
                        float xscale = (float)rc.Width / (float)bitmap.Width;
                        float yscale = (float)rc.Height / (float)bitmap.Height;
                        float xyscale = (xscale < yscale) ? xscale : yscale;
                        int width = (int)(bitmap.Width * xyscale);
                        int height = (int)(bitmap.Height * xyscale);
                        rc.X = (rc.Width - width) / 2;
                        rc.Y = (rc.Height - height) / 2;
                        rc.Width = width;
                        rc.Height = height;
                        e.Graphics.DrawImage(bitmapNew, rc);
                    }
                    else
                    {
                        e.Graphics.DrawImageUnscaled(bitmapNew, 0, 0);
                    }
                }
                finally
                {
                    bitmapNew.Dispose();
                }
            }
        }

        public bool GetContourState()
        {
            return ContourCheckBox.Checked;
        }

        public void DisplayContour(PXCMPointI32[] contour, int blobNumber)
        {
            if (bitmap == null) return;
            lock (this)
            {
                Graphics g = Graphics.FromImage(bitmap);
                using (Pen contourColor = new Pen(Color.Blue, 3.0f))
                {
                    for (int i = 0; i < contour.Length; i++)
                    {
                        int baseX = (int)contour[i].x;
                        int baseY = (int)contour[i].y;

                        if (i + 1 < contour.Length)
                        {
                            int x = (int)contour[i + 1].x;
                            int y = (int)contour[i + 1].y;

                            g.DrawLine(contourColor, new Point(baseX, baseY), new Point(x, y));
                        }
                        else
                        {
                            int x = (int)contour[0].x;
                            int y = (int)contour[0].y;
                            g.DrawLine(contourColor, new Point(baseX, baseY), new Point(x, y));
                        }
                    }
                }
                g.Dispose();
            }
        }

        const float sz2 = 32;
        public void DisplayCursor(int numOfHands, Queue<PXCMPoint3DF32>[] cursorPoints, Queue<PXCMPoint3DF32>[] adaptivePoints, int[] cursorClick)
        {
            if (bitmap == null) return;

            int scaleFactor = 1;
            Graphics g = Graphics.FromImage(bitmap);

            if (CursorMode.Checked)
            {
                Color color = Color.GreenYellow;
                Pen pen = new Pen(color, pSize);

                //i代表第几只手
                for (int i = 0; i < numOfHands; ++i)
                {
                    float sz = 8;

                    /// draw cursor trail
                    if (Cursor.Checked)
                    {
                        for (int j = 0; j < cursorPoints[i].Count; j++)
                        {
                            float greenPart = (float)((Math.Max(Math.Min(cursorPoints[i].ElementAt(j).z / scaleFactor, 0.7), 0.2) - 0.2) / 0.5);
                            pen.Color = Color.FromArgb(255, (int)(255 * (1 - greenPart)), (int)(255 * greenPart), 0);
                            pen.Width = pSize;
                            int x = (int)cursorPoints[i].ElementAt(j).x / scaleFactor;
                            int y = (int)cursorPoints[i].ElementAt(j).y / scaleFactor;
                            g.DrawEllipse(pen, x - sz / 2, y - sz / 2, sz, sz);
                        }
                    }

                    /// draw adaptive trail
                    if (Adaptive.Checked)
                    {
                        for (int j = 0; j < adaptivePoints[i].Count; j++)
                        {
                            float greenPart = (float)((Math.Max(Math.Min(adaptivePoints[i].ElementAt(j).z / scaleFactor, 0.7), 0.2) - 0.2) / 0.5);
                            pen.Color = Color.FromArgb(255, (int)(255 * (1 - greenPart)), (int)(255 * greenPart), 0);
                            pen.Width = pSize;
                            int x = (int)adaptivePoints[i].ElementAt(j).x / scaleFactor;
                            int y = (int)adaptivePoints[i].ElementAt(j).y / scaleFactor;
                            g.DrawEllipse(pen, x - sz / 2, y - sz / 2, sz, sz);
                        }
                    }

                    if (0 < cursorClick[i] && (Cursor.Checked || Adaptive.Checked))
                    {
                        color = Color.LightBlue;
                        pen = new Pen(color, 10.0f);
                        sz = 32;

                        int x = 0, y = 0;
                        if (Cursor.Checked && cursorPoints[i].Count() > 0)
                        {
                            x = (int)cursorPoints[i].ElementAt(cursorPoints[i].Count - 1).x / scaleFactor;
                            y = (int)cursorPoints[i].ElementAt(cursorPoints[i].Count - 1).y / scaleFactor;
                        }
                        else if (Adaptive.Checked && adaptivePoints[i].Count() > 0)
                        {
                            x = (int)adaptivePoints[i].ElementAt(adaptivePoints[i].Count - 1).x / scaleFactor;
                            y = (int)adaptivePoints[i].ElementAt(adaptivePoints[i].Count - 1).y / scaleFactor;
                        }
                        g.DrawEllipse(pen, x - sz / 2, y - sz / 2, sz, sz);
                    }


                    
                }
                pen.Dispose();

                pen = new Pen(Color.Red);
                
                ///Draw beat point
                Queue<PXCMPointF32> bldraw = beatloaction;
                for (int i=0;i<bldraw.Count;i++)
                {
                    int x = (int)bldraw.ElementAt(i).x / scaleFactor;
                    int y = (int)bldraw.ElementAt(i).y / scaleFactor;
                    g.DrawEllipse(pen, x - sz2 / 2, y - sz2 / 2, sz2, sz2);
                }
                pen.Dispose();

                ArrayList als = new ArrayList(pointdraw);
                foreach (PointDraw item in als)
                {
                    int x = (int)item.point.x / scaleFactor;
                    int y = (int)item.point.y / scaleFactor;
                    pen = new Pen(item.color, 10.0f);
                    g.DrawEllipse(pen, x - sz2 / 2, y - sz2 / 2, sz2, sz2);
                    pen.Dispose();
                }
                while (pointdraw.Count > 4)
                    pointdraw.RemoveAt(0);

                adddataflow(numOfHands, cursorPoints);

            }//if cursor mode checked
        }

        static System.Media.SoundPlayer strong = new System.Media.SoundPlayer(@"res\strong.wav");
        static System.Media.SoundPlayer weak = new System.Media.SoundPlayer(@"res\weak.wav");

       System.Threading.Thread analysisThread=new System.Threading.Thread(new System.Threading.ThreadStart(beatanalysis));

        static TimePoint top, buttom, left, right;

        struct TimePoint
        {
            public System.DateTime timestamp;
            public PXCMPointF32 point;
            public TimePoint(PXCMPointF32 p)
            {
                timestamp = System.DateTime.Now;
                point = p;
            }

            public static implicit operator TimePoint(PXCMPointF32 a)
            {
                return new TimePoint(a);
            }
        }

        int topsu, buttomsu, leftsu, rightsu;
        static Queue<FixedPoint> fixedpoint=new Queue<FixedPoint>();
        struct FixedPoint
        {
            public TimePoint point;
            public Direction direction;
            public FixedPoint(TimePoint p,Direction dir)
            {
                point = p;
                direction = dir;
            }


        }
        enum Direction
        {
            top,buttom,left,right
        }

        static bool topupd = true, buttomupd = false, leftupd = false, rightupd = false;
        static bool toplock=false, buttomlock=false, leftlock=true, rightlock=true;
        PXCMPointF32 last;
        static int beat4vote = 0, beat3vote = 0, beat2vote = 0;

        struct PointDraw
        {
            public PXCMPointF32 point;
            public Color color;
            public PointDraw(PXCMPointF32 p,Color c)
            {
                point = p;
                color = c;
            }
        }
        static ArrayList pointdraw = new ArrayList();

        static bool readytostart = false;
        static double intervalms = 0;
        static double volumnrate = 1;
        static double lastamptitude = 0;
        static int errorcount = 0;
        public static void init()
        {
            Console.WriteLine("Init!");
            topupd = true; buttomupd = false; leftupd = false; rightupd = false;
            toplock = false; buttomlock = false; leftlock = true; rightlock = true;
            errorcount = 0;
            dataflow.Clear();
            pointdraw.Clear();
            fixedpoint.Clear();

           if(beat4vote > beat3vote&&beat4vote>beat2vote)
            {
                beat4vote = 1;
                beat3vote = beat2vote = 0;
            }


            else if (beat2vote > beat3vote && beat2vote > beat4vote)
            {
                beat2vote = 2;
                beat3vote = beat4vote = 0;
            }

            else if(beat3vote>beat2vote&&beat3vote>beat4vote)
            {
                beat3vote = 2;
                beat2vote = beat4vote = 0;
            }
           else
            {
                beat4vote = beat3vote = beat2vote = 0;
            }
        }

        static System.DateTime dataflowupd;
        public void adddataflow(int numOfHands, Queue<PXCMPoint3DF32>[] cursorPoints)
        {
            //坐标系测试，结果发现原点在！右上角！
            //x轴横向，y轴纵向
            //pointdraw.Add(new PointDraw(new PXCMPointF32(100, 50), Color.Blue));
            //pointdraw.Add(new PointDraw(new PXCMPointF32(300, 200), Color.Yellow));
            int n = cursorPoints[currentHand].Count();
            if (analysisThread.ThreadState==System.Threading.ThreadState.Unstarted)
            {
                analysisThread.Start();
                return;
            }
            else if(!readytostart)
            {
                if (currentHand < numOfHands && n != 0)
                {
                    Queue<PXCMPoint3DF32> queue = cursorPoints[currentHand];
                    PXCMPointF32 current = reducedepth(queue.Last());
                    dataflow.Enqueue(current);
                    if (dataflow.Count > 1)
                        dataflow.Dequeue();
                }
                return;
            }
            if (currentHand < numOfHands && n != 0)
            {
                Queue<PXCMPoint3DF32> queue = cursorPoints[currentHand];
                PXCMPointF32 current = reducedepth(queue.Last());
                const int leftbuttom_devide= 100;
                dataflow.Enqueue(current);
                dataflowupd = System.DateTime.Now;
                if (dataflow.Count > 4)
                    dataflow.Dequeue();
                //第一个点初始化为最上端点
                if(dataflow.Count==1)
                {
                    top = buttom = left = right = new TimePoint(dataflow.Last());
                    last = current;
                }
                else
                {
                    if (getnorm(subtract(current,last))<10)
                        return;

                    if (current.y > buttom.point.y && !buttomlock) //往下
                    {
                        buttomupd = true;
                        buttom = current;
                        buttomsu = 0;
                    }
                    else
                    {
                        if (buttomupd && !buttomlock)
                            buttomsu += 1;
                        if (buttomsu == 5 && Math.Abs(buttom.point.x-top.point.x) < leftbuttom_devide)
                        {
                            fixedpoint.Enqueue(new FixedPoint(buttom, Direction.buttom));
                            Console.WriteLine("Buttom");
                            pointdraw.Add(new PointDraw(buttom.point, Color.Yellow));
                            buttomlock = true;
                            toplock = false;
                            last = buttom.point;
                            top = subtract(top.point, new PXCMPointF32(0, -70));
                            if (buttomlock&&leftlock)
                            {
                                rightlock = false;
                                right = last;
                            }
                            buttomsu++;
                        }
                        else if (Math.Abs(buttom.point.x - top.point.x) > leftbuttom_devide)
                        {
                            buttomsu = 0;
                        }
                    }
                    if (current.x < right.point.x && !rightlock) //往右
                    {
                        rightupd = true;
                        right = current;
                        rightsu = 0;
                    }
                    else
                    {
                        if (rightupd && !rightlock)
                            rightsu += 1;
                        if (rightsu == 5)
                        {
                            rightlock = true;
                            fixedpoint.Enqueue(new FixedPoint(right, Direction.right));
                            pointdraw.Add(new PointDraw(right.point, Color.Blue));
                            Console.WriteLine("right");
                            toplock = false;
                            buttomlock = true;
                            last = right.point;
                            top = last;
                            rightsu++;
                        }
                    }

                    if (current.x > left.point.x && !leftlock)
                    {
                        leftupd = true;
                        left = current;
                        leftsu = 0;
                    }
                    else
                    {
                        if (leftupd && !leftlock)
                            leftsu += 1;
                        if (leftsu == 5 && Math.Abs(left.point.x - top.point.x) > leftbuttom_devide)
                        {
                            leftlock = true;
                            toplock = true;
                            fixedpoint.Enqueue(new FixedPoint(left, Direction.left));
                            pointdraw.Add(new PointDraw(left.point, Color.Green));
                            rightlock = false;
                            buttomlock = true;
                            Console.WriteLine("left");
                            last = left.point;
                            right = last;
                            leftsu++;
                        }
                        else if (Math.Abs(left.point.x - top.point.x) < leftbuttom_devide)
                            leftsu = 0;
                    }

                    if (current.y < top.point.y && !toplock)
                    {
                        topupd = true;
                        top = current;
                        topsu = 0;
                    }
                    else
                    {
                        if(topupd && !toplock)
                            topsu += 1;
                        if (topsu == 5)
                        {
                            toplock = true;
                            fixedpoint.Enqueue(new FixedPoint(top, Direction.top));
                            pointdraw.Add(new PointDraw(top.point, Color.Red));
                            Console.WriteLine("top");
                            last = top.point;
                            leftlock = buttomlock = false;
                            left = buttom = last;
                            topsu++;
                        }
                            
                    }
                    

                    while (fixedpoint.Count>0 && fixedpoint.First().direction != Direction.top)
                        fixedpoint.Dequeue();

                    if (fixedpoint.Count>=3)
                    {
                        string pattern = "";
                        foreach (FixedPoint item in fixedpoint)
                        {
                            if (item.direction == Direction.top)
                            {
                                pattern += "T";
                            }
                            else if (item.direction == Direction.buttom)
                            {
                                pattern += "B";
                            }
                            if (item.direction == Direction.left)
                            {
                                pattern += "L";
                            }
                            if (item.direction == Direction.right)
                            {
                                pattern += "R";
                            }
                        }

                        if (pattern=="TBLR")
                        {
                            beat4vote = Math.Min(beat4vote + 1, 5);
                            if(beat4vote==5)
                            {
                                beat3vote = Math.Max(beat3vote - 1, 0);
                                beat2vote = Math.Max(beat2vote - 1, 0);
                            }
                        }

                        else if (pattern=="TLR")
                        {
                            beat3vote = Math.Min(beat3vote + 1, 5);
                            if (beat3vote == 5)
                            {
                                beat4vote = Math.Max(beat4vote - 1, 0);
                                beat2vote = Math.Max(beat2vote - 1, 0);
                            }
                        }

                        else if (pattern=="TBTB")
                        {
                            beat2vote = Math.Min(beat2vote + 1, 5);
                            if(beat2vote==5)
                            {
                                beat3vote = Math.Max(beat3vote - 1, 0);
                                beat4vote = Math.Max(beat4vote - 1, 0);
                            }
                        }

                        const double updrate = 0.2;
                        const double vupdrate = 0.2;
                        if (beat4vote>beat3vote&& beat4vote>beat2vote && pattern=="TBLR")
                        {
                            TimeSpan interval = new TimeSpan();
                            for (int i=1;i<fixedpoint.Count;i++)
                            {
                                interval += fixedpoint.ElementAt(i).point.timestamp - fixedpoint.ElementAt(i-1).point.timestamp;
                            }
                            if (intervalms == 0)
                                intervalms = interval.TotalMilliseconds / 3;
                            else
                                intervalms = updrate * interval.TotalMilliseconds / 3 + (1-updrate) * intervalms;
                            Console.WriteLine(pattern + "  Interval Updated! New interval:" + intervalms);

                            double amptitude = getnorm(subtract(fixedpoint.ElementAt(1).point.point, fixedpoint.ElementAt(0).point.point));
                            if (lastamptitude == 0)
                                lastamptitude = amptitude;
                            else
                            {
                                double ratio = amptitude / lastamptitude;
                                ratio = (ratio - 1) * vupdrate + ratio;
                                volumnrate = ratio;
                                lastamptitude = amptitude;
                            }
                            fixedpoint.Clear();
                        }

                        if (beat3vote > beat4vote && beat3vote > beat2vote && pattern=="TLR")
                        {
                            TimeSpan interval = new TimeSpan();
                            for (int i = 1; i < fixedpoint.Count; i++)
                            {
                                interval += fixedpoint.ElementAt(i).point.timestamp - fixedpoint.ElementAt(i - 1).point.timestamp;
                            }
                            if (intervalms == 0)
                                intervalms = interval.TotalMilliseconds / 2;
                            else
                                intervalms = updrate * interval.TotalMilliseconds / 2 + (1 - updrate) * intervalms;
                            Console.WriteLine(pattern + "  Interval Updated! New interval:" + intervalms);

                            double amptitude = 0;
                            amptitude += getnorm(subtract(fixedpoint.ElementAt(1).point.point, fixedpoint.ElementAt(0).point.point));
                            amptitude += getnorm(subtract(fixedpoint.ElementAt(2).point.point, fixedpoint.ElementAt(1).point.point));
                            amptitude /= 2;
                            if (lastamptitude == 0)
                                lastamptitude = amptitude;
                            else
                            {
                                double ratio = amptitude / lastamptitude;
                                ratio = (ratio - 1) * vupdrate + ratio;
                                volumnrate = ratio;
                                lastamptitude = amptitude;
                            }

                            
                        }

                        if (pattern == "TLR")
                            fixedpoint.Clear();

                        if (beat2vote>beat4vote && beat2vote>beat3vote && pattern=="TBTB")
                        {
                            TimeSpan interval = new TimeSpan();
                            for (int i = 1; i < fixedpoint.Count; i++)
                            {
                                interval += fixedpoint.ElementAt(i).point.timestamp - fixedpoint.ElementAt(i - 1).point.timestamp;
                            }
                            if (intervalms == 0)
                                intervalms = interval.TotalMilliseconds / 3;
                            else
                                intervalms = updrate * interval.TotalMilliseconds / 3 + (1 - updrate) * intervalms;
                            Console.WriteLine(pattern + "  Interval Updated! New interval:" + intervalms);

                            double amptitude = 0;
                            amptitude += getnorm(subtract(fixedpoint.ElementAt(1).point.point, fixedpoint.ElementAt(0).point.point));
                            amptitude += getnorm(subtract(fixedpoint.ElementAt(2).point.point, fixedpoint.ElementAt(1).point.point));
                            amptitude += getnorm(subtract(fixedpoint.ElementAt(3).point.point, fixedpoint.ElementAt(2).point.point));
                            amptitude /= 3;
                            if (lastamptitude == 0)
                                lastamptitude = amptitude;
                            else
                            {
                                double ratio = amptitude / lastamptitude;
                                ratio = (ratio - 1) * vupdrate + ratio;
                                volumnrate = ratio;
                                lastamptitude = amptitude;
                            }

                            fixedpoint.Clear();
                        }


                        //if(pattern!="TBTB"&&pattern!="TBLR"&&pattern!="TLR")
                        //{
                        //    errorcount++;
                        //    if (errorcount>100)
                        //    {
                        //        readytostart = false;
                        //        init();
                        //        Console.WriteLine("Move cursor into the orange circle to restart");
                        //    }
                        //}
                        //else
                        //{
                        //    errorcount = 0;
                        //}

                        if (fixedpoint.Count>=4)
                            fixedpoint.Clear();
                    }
                }

            }
            else
            {
                //选择另一只手
                for (int handptr = 0; handptr < numOfHands; handptr++)
                {
                    int maxpointcount = 0;
                    if (cursorPoints[handptr].Count > maxpointcount)
                    {
                        currentHand = handptr;
                        maxpointcount = cursorPoints[handptr].Count;
                    }
                }
                //analysisThread.Abort();
                readytostart = false;
                init();
            }
        }

        
        
        struct DataFrame
        {
            public PXCMPointF32 loc;
            public PXCMPoint3DF32 loc3d;
            public PXCMPointF32? vec_v;
            public PXCMPoint3DF32? vec_v3d;
            public double v;
            public double vangle;
            public PXCMPointF32? vec_acc;
            public PXCMPoint3DF32? vec_acc3d;
            public double acc;
            public double acc_diff;
            public DataFrame(PXCMPoint3DF32 point,DataFrame? previous1=null, DataFrame? previous2=null)
            {
                loc3d = point;
                loc = reducedepth(point);
                if (previous1 != null)
                {
                    DataFrame prev = (DataFrame)previous1;
                    vec_v3d = subtract(point, prev.loc3d);
                    vec_v = reducedepth(vec_v3d.Value);
                    v = getnorm(vec_v.Value);
                    vangle = Math.Atan2(vec_v.Value.y, vec_v.Value.x)/Math.PI * 180;

                    if(previous2!=null)
                    {
                        PXCMPoint3DF32 v0 = previous2.Value.vec_v3d.Value;
                        vec_acc3d = subtract(vec_v3d.Value, v0);
                        vec_acc = reducedepth(vec_acc3d.Value);
                        const double a = 1;
                        double pacc = previous1.Value.acc;
                        if (!double.IsNaN(pacc))
                            acc = getnorm(vec_acc.Value) * a + pacc * (1 - a);
                        else
                            acc = getnorm(vec_acc.Value);
                        acc_diff = v - previous1.Value.v;
                    }
                    else
                    {
                        vec_acc = null;
                        vec_acc3d = null;
                        acc = double.NaN;
                        acc_diff = double.NaN;
                    }
                }
                else
                {
                    vec_v = null;
                    vec_v3d = null;
                    vec_acc = null;
                    vec_acc3d = null;
                    v = double.NaN;
                    vangle = double.NaN;
                    acc = double.NaN;
                    acc_diff = double.NaN;
                }
            }
        }

        static int currentHand = 0;
        static Queue<PXCMPointF32> beatloaction = new Queue<PXCMPointF32>();
        static Queue<PXCMPointF32> dataflow = new Queue<PXCMPointF32>();
        static Queue<DataFrame> trace=new Queue<DataFrame>();

        public static void beatanalysis()
        {
            System.Threading.Thread.Sleep(1000);
            Console.WriteLine("3");
            System.Threading.Thread.Sleep(1000);
            Console.WriteLine("2");
            System.Threading.Thread.Sleep(1000);
            Console.WriteLine("1");
            System.Threading.Thread.Sleep(1000);
            Console.WriteLine("Go!");
            dataflow.Clear();
            readytostart = true;

            int flowstopcount = 0;
            System.DateTime dtfupd = dataflowupd;
            int beatcount = 0;
            const int defaultinterval = 500;
            while(true)
            {
                if (intervalms==0)
                    System.Threading.Thread.Sleep(defaultinterval);
                else
                {
                    if (beat4vote>beat3vote && beat4vote>beat2vote)
                    {
                        if (beatcount % 4 == 0) strong.Play();
                        else
                            weak.Play();
                        beatcount = ++beatcount % 4;
                    }
                    else if (beat3vote > beat4vote && beat3vote > beat2vote)
                    {
                        if (beatcount % 3 == 0) strong.Play();
                        else
                            weak.Play();
                        
                        beatcount = ++beatcount % 3;
                    }
                    else if(beat2vote>beat4vote&&beat2vote>beat3vote)
                    {
                        if (beatcount % 2 == 0) strong.Play();
                        else
                            weak.Play();
                        beatcount = ++beatcount % 2;
                    }

                   // Console.WriteLine((int)intervalms);
                    System.Threading.Thread.Sleep((int)intervalms);
                }
                if (dataflowupd==dtfupd && readytostart)
                {
                    flowstopcount += (intervalms == 0 ? defaultinterval : (int)intervalms);
                }                
                else if (readytostart)
                {
                    dtfupd = dataflowupd;
                    flowstopcount = 0;
                }
                else if (!readytostart)
                {
                    PXCMPointF32 st = top.point;
                    pointdraw.Add(new PointDraw(st, Color.Orange));
                    if (dataflow.Count>=1 && getnorm(subtract(st, dataflow.Last())) < sz2)
                    {
                        System.Threading.Thread.Sleep(100);
                        readytostart = true;
                        pointdraw.Clear();
                    }
                        
                }

                if (flowstopcount>=3000 && readytostart)
                {
                    readytostart = false;
                    flowstopcount = 0;
                    init();
                    Console.WriteLine("Move cursor into the orange circle to restart");
                }
                    
            }

        }
        //{
        //    try
        //    {
        //        System.Threading.Thread.Sleep(1000);
        //        Console.WriteLine("3");
        //        System.Threading.Thread.Sleep(1000);
        //        Console.WriteLine("2");
        //        System.Threading.Thread.Sleep(1000);
        //        Console.WriteLine("1");
        //        System.Threading.Thread.Sleep(1000);
        //        Console.WriteLine("Go!");
        //        dataflow.Clear();

        //        System.IO.StreamWriter output = new System.IO.StreamWriter("velocity.txt");

        //        Color color = Color.Red;
        //        Pen pen = new Pen(color, 10.0f);
        //        float sz = 32;
        //        int scaleFactor = 1;
        //        int idlecount = 0;
        //        Console.WriteLine("Analysis Starting...");
        //        while (analysison == true)
        //        {
        //            int n_flow = dataflow.Count;
        //            int n_trace = trace.Count;

        //            //同步当前dataflow的元素到trace中
        //            if (n_flow>n_trace)
        //            {
        //                idlecount = 0;
        //                //补入未分析的数据
        //                for (int i=n_trace;i<n_flow;i++)
        //                {
        //                    if (trace.Count>=2)
        //                    {
        //                        trace.Enqueue(new DataFrame(dataflow.ElementAt(i), trace.Last(), trace.ElementAt(trace.Count - 1)));
        //                    }
        //                    else if(trace.Count==1)
        //                    {
        //                        trace.Enqueue(new DataFrame(dataflow.ElementAt(i), trace.Last()));
        //                    }
        //                    else
        //                    {
        //                        trace.Enqueue(new DataFrame(dataflow.ElementAt(i)));
        //                    }

        //                }
        //                Queue<double> diffqueue = new Queue<double>();


        //                //队列分析




        //                while(trace.Count>100)
        //                {
        //                    trace.Dequeue();
        //                    dataflow.Dequeue();
        //                }
        //            }
        //            else
        //            {
        //                System.Threading.Thread.Sleep(100);
        //                idlecount += 50;
        //                if (idlecount==1000)
        //                {
        //                    dataflow.Clear();
        //                    trace.Clear();
        //                    beatloaction.Clear();
        //                    idlecount = 0;
        //                }
        //            }

        //        }
        //    }
        //    catch (System.Threading.ThreadAbortException)
        //    {
        //        Console.WriteLine("Aborting analysis...");
        //    }

        //}

        public static PXCMPoint3DF32 subtract(PXCMPoint3DF32 left, PXCMPoint3DF32 right)
        {
            return new PXCMPoint3DF32(left.x - right.x, left.y - right.y, left.z - right.z);
        }

        public static PXCMPointF32 subtract(PXCMPointF32 left, PXCMPointF32 right)
        {
            return new PXCMPointF32(left.x - right.x, left.y - right.y);
        }

        public static double getnorm(PXCMPointF32 a2)
        {
            return Math.Sqrt(a2.x * a2.x + a2.y * a2.y);
        }

        public static string tostring(PXCMPointF32 a)
        {
            return "[" + a.x + "," + a.y + "]";
        }


        public static PXCMPointF32 reducedepth(PXCMPoint3DF32 a)
        {
            return new PXCMPointF32(a.x, a.y);
        }

        public static double getdegree(PXCMPointF32 a)
        {
            return Math.Atan2(a.y, a.x) / Math.PI * 180;
        }


        public void DisplayExtremities(int numOfHands, PXCMHandData.ExtremityData[][] extremitites = null)
        {
            if (bitmap == null) return;
            if (extremitites == null) return;
            
            int scaleFactor = 1;
            Graphics g = Graphics.FromImage(bitmap);

            float sz = 8;
            if (Labelmap.Checked)
                sz = 4;

            Pen pen = new Pen(Color.Red, 3.0f);
		    for (int i = 0; i < numOfHands; ++i) 
		    {
			    for (int j = 0 ;j < PXCMHandData.NUMBER_OF_EXTREMITIES; ++j) 			
			    {
                    int x = (int)extremitites[i][j].pointImage.x / scaleFactor;
                    int y = (int)extremitites[i][j].pointImage.y / scaleFactor;
                    g.DrawEllipse(pen, x - sz / 2, y - sz / 2, sz, sz);
			    }
		    }
            pen.Dispose();
        }

        public void DisplayJoints(PXCMHandData.JointData[][] nodes, int numOfHands)
        {
            if (bitmap == null) return;
            if (nodes == null) return;

            if (Joints.Checked || Skeleton.Checked)
            {
                lock (this)
                {
                    int scaleFactor = 1;

                    Graphics g = Graphics.FromImage(bitmap);

                    using (Pen boneColor = new Pen(Color.DodgerBlue, 3.0f))
                    {
                        for (int i = 0; i < numOfHands; i++)
                        {
                            if (nodes[i][0] == null) continue;
                            int baseX = (int) nodes[i][0].positionImage.x/scaleFactor;
                            int baseY = (int) nodes[i][0].positionImage.y/scaleFactor;

                            int wristX = (int) nodes[i][0].positionImage.x/scaleFactor;
                            int wristY = (int) nodes[i][0].positionImage.y/scaleFactor;

                            if (Skeleton.Checked)
                            {
                                for (int j = 1; j < 22; j++)
                                {
                                    if (nodes[i][j] == null) continue;
                                    int x = (int) nodes[i][j].positionImage.x/scaleFactor;
                                    int y = (int) nodes[i][j].positionImage.y/scaleFactor;

                                    if (nodes[i][j].confidence <= 0) continue;

                                    if (j == 2 || j == 6 || j == 10 || j == 14 || j == 18)
                                    {

                                        baseX = wristX;
                                        baseY = wristY;
                                    }

                                    g.DrawLine(boneColor, new Point(baseX, baseY), new Point(x, y));
                                    baseX = x;
                                    baseY = y;
                                }
                            }

                            if (Joints.Checked)
                            {
                                using (
                                    Pen red = new Pen(Color.Red, 3.0f),
                                        black = new Pen(Color.Black, 3.0f),
                                        green = new Pen(Color.Green, 3.0f),
                                        blue = new Pen(Color.Blue, 3.0f),
                                        cyan = new Pen(Color.Cyan, 3.0f),
                                        yellow = new Pen(Color.Yellow, 3.0f),
                                        orange = new Pen(Color.Orange, 3.0f))
                                {
                                    Pen currnetPen = black;

                                    for (int j = 0; j < PXCMHandData.NUMBER_OF_JOINTS; j++)
                                    {
                                        float sz = 4;
                                        if (Labelmap.Checked)
                                            sz = 2;

                                        int x = (int) nodes[i][j].positionImage.x/scaleFactor;
                                        int y = (int) nodes[i][j].positionImage.y/scaleFactor;

                                        if (nodes[i][j].confidence <= 0) continue;

                                        //Wrist
                                        if (j == 0)
                                        {
                                            currnetPen = black;
                                        }

                                        //Center
                                        if (j == 1)
                                        {
                                            currnetPen = red;
                                            sz += 4;
                                        }

                                        //Thumb
                                        if (j == 2 || j == 3 || j == 4 || j == 5)
                                        {
                                            currnetPen = green;
                                        }
                                        //Index Finger
                                        if (j == 6 || j == 7 || j == 8 || j == 9)
                                        {
                                            currnetPen = blue;
                                        }
                                        //Finger
                                        if (j == 10 || j == 11 || j == 12 || j == 13)
                                        {
                                            currnetPen = yellow;
                                        }
                                        //Ring Finger
                                        if (j == 14 || j == 15 || j == 16 || j == 17)
                                        {
                                            currnetPen = cyan;
                                        }
                                        //Pinkey
                                        if (j == 18 || j == 19 || j == 20 || j == 21)
                                        {
                                            currnetPen = orange;
                                        }


                                        if (j == 5 || j == 9 || j == 13 || j == 17 || j == 21)
                                        {
                                            sz += 4;
                                        }

                                        g.DrawEllipse(currnetPen, x - sz/2, y - sz/2, sz, sz);
                                    }
                                }
                            }
                        }
                    }
                    g.Dispose();
                }
            }
        }

    

        private delegate void UpdatePanelDelegate();
        public void UpdatePanel()
        {

            Panel2.Invoke(new UpdatePanelDelegate(delegate()
            { Panel2.Invalidate();
            }));

        }

        private void simpleToolStripMenuItem_Click(object sender, EventArgs e)
        {
            RadioCheck(sender, "Pipeline");
        }

        private void advancedToolStripMenuItem_Click(object sender, EventArgs e)
        {
            RadioCheck(sender, "Pipeline");
        }

        private void timer_Tick(object sender, EventArgs e)
        {
            
        }

        private void Live_Click(object sender, EventArgs e)
        {
            Playback.Checked = Record.Checked = false;
            Live.Checked = true;
        }

        private void Playback_Click(object sender, EventArgs e)
        {
            Live.Checked = Record.Checked = false;
            Playback.Checked = true;

            using (OpenFileDialog ofd = new OpenFileDialog())
            {
                ofd.Filter = "RSSDK clip|*.rssdk|Old format clip|*.pcsdk|All files|*.*";
                ofd.CheckFileExists = true;
                ofd.CheckPathExists = true;
                filename = (ofd.ShowDialog() == DialogResult.OK) ? ofd.FileName : null;
            }
        }

        public bool GetPlaybackState()
        {
            return Playback.Checked;
        }



        private void Record_Click(object sender, EventArgs e)
        {
            Live.Checked = Playback.Checked = false;
            Record.Checked = true;
            SaveFileDialog sfd = new SaveFileDialog();
            sfd.Filter = "RSSDK clip|*.rssdk|All Files|*.*";
            sfd.CheckPathExists = true;
            sfd.OverwritePrompt = true;
            sfd.AddExtension = true;
            filename = (sfd.ShowDialog() == DialogResult.OK) ? sfd.FileName : null;
        }

        public bool GetRecordState()
        {
            return Record.Checked;
        }

        public string GetFileName()
        {
            return filename;
        }

        private void cmbGesturesList_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (cmbGesturesList.SelectedItem != null)
            {
                gestureName = cmbGesturesList.SelectedItem.ToString();
            }
            else
            {
                gestureName = string.Empty;
            }
        }

        public string GetGestureName()
        {
            return gestureName;
        }

        private void Depth_CheckedChanged(object sender, EventArgs e)
        {
            ContourCheckBox.Enabled = false;
        }

        private void Labelmap_CheckedChanged(object sender, EventArgs e)
        {
            ContourCheckBox.Enabled = true;
        }

        private void TrackingModeChanged(object sender, EventArgs e)
        {
           
            Labelmap.Enabled = true;

            Joints.Enabled = false;
            Joints.Checked = false;
            Skeleton.Enabled = false;
            Skeleton.Checked = false;
            Cursor.Enabled = false;
            Cursor.Checked = false;
            Adaptive.Enabled = false;
            Adaptive.Checked = false;
            Extremities.Enabled = false;
            Extremities.Checked = false;

            if (FullHandMode.Checked)
            {
                Labelmap.Enabled = true;
                Depth.Enabled = true;

                Joints.Enabled = true;
                Joints.Checked = true;
                Skeleton.Enabled = true;
                Skeleton.Checked = true;
                Extremities.Checked = false;
                Extremities.Enabled = true;
            }

            if (CursorMode.Checked)
            {
                Labelmap.Enabled = false;
                Labelmap.Checked = false;
                Depth.Enabled = false;

                Cursor.Checked = true;
                Cursor.Enabled = true;
                //Adaptive.Checked = true;
                Adaptive.Enabled = true;
            }
        }

        private void infoTextBox_TextChanged(object sender, EventArgs e)
        {

        }
    }
}
