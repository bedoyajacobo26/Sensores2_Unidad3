using System.Collections;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using UnityEngine;
using UnityEngine.UI;

public class UDPapp: MonoBehaviour
{
    private static UDPapp instance;
    private Thread receiveThread;
    private UdpClient receiveClient;
    private IPEndPoint receiveEndPoint;

    CONFIGURACION DontDestroy;
    public string ip = "";
    public int receivePort;
    int UDP_Port;
    string UDP_ID;
    private bool isInitialized;
    private Queue receiveQueue;


    byte[] message;
    

    public Text segundo;
    public Text minuto;
    public Text hora;
    public Text diaSemana;
    public Text diaMes;
    public Text mes;
    public Text año;

    public Text humedad;
    public Text temperatura;
    public Text altitud;
    public Text presion;


    

    private void Awake()
    {
        Initialize();

        DontDestroy = GameObject.Find("DD").GetComponent<CONFIGURACION>();
        UDP_Port = int.Parse(DontDestroy.vPuerto_esp32);
        UDP_ID = DontDestroy.vID_esp32;

        ip = UDP_ID;
        receivePort = UDP_Port;
    }

    private void Start()
    {
      
    }

    private void Initialize()
    {
        instance = this;
        receiveEndPoint = new IPEndPoint(IPAddress.Any, receivePort);
        receiveClient = new UdpClient(receivePort);
        receiveQueue = Queue.Synchronized(new Queue());
        receiveThread = new Thread(new ThreadStart(ReceiveDataListener));
        receiveThread.IsBackground = true;
        receiveThread.Start();
        isInitialized = true;
        Debug.Log("init");
    }

    private void ReceiveDataListener()
    {
        Debug.Log("r_data");
        while (true)
        {
           
            byte[] data = receiveClient.Receive(ref receiveEndPoint);


            receiveQueue.Enqueue(data);

          //  SerializeMessage(data);
        }
    }

    private void SerializeMessage(byte[] data)
    {
        try
        {
            for (int i = 0; i < data.Length ; i++)
            {
                receiveQueue.Enqueue(data[i]);
            }             
        }
        catch (System.Exception e)
        {
            Debug.Log(e.ToString());
        }
    }

    private void OnDestroy()
    {
        TryKillThread();
    }

    private void OnApplicationQuit()
    {
        TryKillThread();
    }

    private void TryKillThread()
    {
        if (isInitialized)
        {
            receiveThread.Abort();
            receiveThread = null;
            receiveClient.Close();
            receiveClient = null;
            Debug.Log("Thread killed");
            isInitialized = false;
        }
    }

    void Update()
    {
        if (receiveQueue.Count != 0)
        {

           

            byte[] message = (byte[])receiveQueue.Dequeue();



            if (message == null)
                return;




            byte seconds = message[0];
            byte minuts = message[1];
            byte hours = message[2];
            byte dayOfWeek = message[3];
            byte dayOfMonth = message[4];
            byte month = message[5];
            byte year = message[6];

            byte[] arr_temp = new[] { message[7], message[8], message[9], message[10] };
            byte[] arr_pres = new[] { message[11], message[12], message[13], message[14] };
            byte[] arr_alti = new[] { message[15], message[16], message[17], message[18] };
            byte[] arr_humi = new[] { message[19], message[20], message[21], message[22] };

            float f_temperatura = System.BitConverter.ToSingle(arr_temp, 0);
            float f_presion = System.BitConverter.ToSingle(arr_pres, 0);
            float f_altitud = System.BitConverter.ToSingle(arr_alti, 0);
            float f_humedad = System.BitConverter.ToSingle(arr_humi, 0);

            segundo.text = seconds.ToString();
            minuto.text = minuts.ToString();
            hora.text = hours.ToString();
            diaSemana.text = dayOfWeek.ToString();
            diaMes.text = dayOfMonth.ToString();
            mes.text = month.ToString();
            año.text = year.ToString();

            temperatura.text = f_temperatura.ToString();
            presion.text = f_presion.ToString();
            altitud.text = f_altitud.ToString();
            humedad.text = f_humedad.ToString();

            Debug.Log(" Seconds " + message[0] + " minuts " + message[1] + " hours " + message[2] + " dayOfWeek " +
                 message[3] + " dayOfMonth " + message[4] + " month " + message[5] + " year " + message[6] + " temperature " +
                  f_temperatura + " pressure " + f_presion + " altitude " + f_altitud + " humidity " + f_humedad);
        }

    }

}