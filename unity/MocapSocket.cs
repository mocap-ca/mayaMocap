using UnityEngine;
using System.Collections;
using System;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using System.Text;
using System.Collections.Generic;



public class MocapSocket : MonoBehaviour {

	Byte[] data;
	UdpClient clientSocket;
	IPEndPoint groupEP;

	string buffer;
	string infoMessage;

	public int listenPort;
	Boolean toggleDisplay;

	string[] transformList;
	Dictionary< string, GameObject > objects = new Dictionary< string, GameObject> ();
	Dictionary< string, Quaternion > rotationOffsets = new Dictionary<string, Quaternion> ();
	Dictionary< string, Quaternion > initialOrientation = new Dictionary<string, Quaternion> ();

	int frameCount = 0;
	double dt = 0.0;
	double fps = 0.0;
	double updateRate = 4.0;  // 4 updates per sec.

	
	// Use this for initialization
	void Start () {


		toggleDisplay = false;

		transformList =  new string[] {"Hips",  
			"LeftUpLeg", "LeftLeg", "LeftFoot", "LeftToeBase", 
			"RightUpLeg", "RightLeg", "RightFoot", "RightToeBase", 
			"Spine", "Spine1", "Spine2", "Spine3",
			"Neck", "Neck1", "Head",     
			"LeftShoulder", "LeftArm", "LeftForeArm", "LeftHand", 
			"RightShoulder", "RightArm", "RightForeArm", "RightHand", "VCAM"
		};

		foreach (string s in transformList) 
		{
			GameObject o = GameObject.Find (s);

			if (o == null)
			{
				Debug.Log ("Could not find object: " + s);
			}
			else
			{
				objects.Add (s, o);
				rotationOffsets.Add ( s, Quaternion.identity );
				initialOrientation.Add ( s, o.transform.localRotation );
			}
		}

		data = new Byte[8192];

		infoMessage = "Hi";

		Connect();

	}

	void Connect()
	{
		Debug.Log ("Connect");

		//if (clientSocket != null && clientSocket.Connected) Disconnect();

		// Create Socket
		clientSocket = new UdpClient(listenPort);
		groupEP      = new IPEndPoint(IPAddress.Any,listenPort);
	
	}

	void Disconnect()
	{
		if (clientSocket.Client.Connected) clientSocket.Close();
	}
	 
	 
	void OnGUI()
	{
		if (!clientSocket.Client.Connected)
			GUI.Label (new Rect (Screen.width - 150, 5, 150, 40), "Not Connected");
			
		if(toggleDisplay)
			GUI.Label(	new Rect(5, 5, 	Screen.width, Screen.height), infoMessage);

		//if(GUI.Button (new Rect(120,10,100,20), "disconnect"))
		//{
		//	currentState = 2;
		//}
	}

	public static Quaternion QuaternionFromMatrix(Matrix4x4 m) {
		// Adapted from: http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/index.htm
		Quaternion q = new Quaternion();
		q.w = Mathf.Sqrt( Mathf.Max( 0, 1 + m[0,0] + m[1,1] + m[2,2] ) ) / 2; 
		q.x = Mathf.Sqrt( Mathf.Max( 0, 1 + m[0,0] - m[1,1] - m[2,2] ) ) / 2; 
		q.y = Mathf.Sqrt( Mathf.Max( 0, 1 - m[0,0] + m[1,1] - m[2,2] ) ) / 2; 
		q.z = Mathf.Sqrt( Mathf.Max( 0, 1 - m[0,0] - m[1,1] + m[2,2] ) ) / 2; 
		q.x *= Mathf.Sign( q.x * ( m[2,1] - m[1,2] ) );
		q.y *= Mathf.Sign( q.y * ( m[0,2] - m[2,0] ) );
		q.z *= Mathf.Sign( q.z * ( m[1,0] - m[0,1] ) );
		return q;
	}

	void Update()

	{
		if (Input.GetKeyDown ("z"))
				toggleDisplay = !toggleDisplay;
	}
			
	// Update is called once per frame
	void FixedUpdate ()
	{
		if (clientSocket.Available == 0)
						return;

		// Frame rate calculator

		dt += Time.deltaTime;
		if (dt > 1.0/updateRate)
		{
			fps = frameCount / dt ;
			frameCount = 0;
			dt -= 1.0/updateRate;
		}

		try
		{
			byte[] bytes = clientSocket.Receive( ref groupEP );

			Parser parser = new Parser();
			parser.parseItems ( bytes );

			infoMessage = "Bytes " + bytes.Length;
			infoMessage += "  Segments: " + parser.segments.Count;

			for(int i =0; i < parser.segments.Count; i++)
			{
				MocapSegment segment = parser.segments[i];

				infoMessage += "\n" + segment.name;

				if(objects.ContainsKey (segment.name))
				{
					Quaternion localOrientation  =  segment.GetOrientation();
					
					// Zero off the rotation
					if (Input.GetKeyDown ("1"))
					{
						rotationOffsets[segment.name] = Quaternion.Inverse ( localOrientation );
					}
					
					GameObject o = objects[segment.name];
					o.transform.localRotation = localOrientation * rotationOffsets[segment.name]; 
					o.transform.localPosition = segment.GetTranslation(0.1f);
					
					String dbg = "SEG:" + segment.name + "\n";
					
					Vector3    eul = localOrientation.eulerAngles;
					Vector3    ofs = rotationOffsets[segment.name].eulerAngles;
					Vector3    ofi = initialOrientation[segment.name].eulerAngles;
					Vector3    eu2 = o.transform.localRotation.eulerAngles;
					Vector3    pos = o.transform.localPosition;
					
					float px = pos[0] * 100;
					float py = pos[1] * 100;
					float pz = pos[2] * 100;
					
					dbg += "POS:" + px.ToString ("0.00") + "\t\t" + py.ToString ("0.00") + "\t\t" + pz.ToString("0.00") + "\n";
					dbg += "L:  " + eul.x.ToString ("0.00") + "\t\t" + eul.y.ToString ("0.00") + "\t\t" + eul.z.ToString("0.00") + "\n";
					//Debug.Log (dbg);

					infoMessage += "  tx: " + segment.tx;
					infoMessage += "  ty: " + segment.ty;
					infoMessage += "  tz: " + segment.tz;
					
					//frameDone = true;
				}
			}
				
			/*

			int subjects = 0;
			if(!int.TryParse ( lineItems[0], out subjects))
			{
				infoMessage = "Invalid data while parsing";
				return;
			}
			int line = 1;
			bool frameDone = false;
			for(int i=0; i < subjects && line < lineItems.Length; i++)
			{
				string[] subjectSplit = lineItems[line++].Split ('\t');
				string subjectName = subjectSplit[0];
				int    noSegments  = Convert.ToInt32 (subjectSplit[1]);
				int    noMarkers   = Convert.ToInt32 (subjectSplit[2]);
				//infoMessage += "SUB: " + subjectName + "\n";


				for(int j=0; j < noSegments && line < lineItems.Length; j++)
				{
					string[] segmentSplit = lineItems[line++].Split('\t');

					if(segmentSplit.Length != 8)
					{
						infoMessage += "Segments: " + segmentSplit.Length;
						continue;
					}

					string segmentName = subjectName + "_" + segmentSplit[0];
					float[] tr = new float[3];
					float[] lo = new float[4];
					for(int k=0; k < 3; k++) tr[k] = float.Parse (segmentSplit[k+1]);
					for(int k=0; k < 4; k++) lo[k] = float.Parse (segmentSplit[k+4]);

					if(objects.ContainsKey (segmentName))
					{
						Quaternion localOrientation  =  new Quaternion(lo[0], lo[1], lo[2], lo[3]);

						// Zero off the rotation
						if (Input.GetKeyDown ("1"))
						{
							rotationOffsets[segmentName] = Quaternion.Inverse ( localOrientation );
						}

						GameObject o = objects[segmentName];
						o.transform.localRotation = localOrientation * rotationOffsets[segmentName]; 
						o.transform.localPosition = new Vector3(tr[0] / 10, tr[1] / 10, tr[2] / 10);

						String dbg = "SEG:" + segmentName + "\n";

						Vector3    eul = localOrientation.eulerAngles;
						Vector3    ofs = rotationOffsets[segmentName].eulerAngles;
						Vector3    ofi = initialOrientation[segmentName].eulerAngles;
						Vector3    eu2 = o.transform.localRotation.eulerAngles;
						Vector3    pos = o.transform.localPosition;
						
						float px = pos[0] * 100;
						float py = pos[1] * 100;
						float pz = pos[2] * 100;
						
						dbg += "POS:" + px.ToString ("0.00") + "\t\t" + py.ToString ("0.00") + "\t\t" + pz.ToString("0.00") + "\n";
						dbg += "L:  " + eul.x.ToString ("0.00") + "\t\t" + eul.y.ToString ("0.00") + "\t\t" + eul.z.ToString("0.00") + "\n";
						//Debug.Log (dbg);

						frameDone = true;
					}

				} // done segments

				line += noMarkers;
			}

			infoMessage += "FPS:" + fps.ToString ("0.00");

			*/


			//if(frameDone) frameCount++;




			/*
			float f = float.Parse(last, System.Globalization.CultureInfo.InvariantCulture);
			Debug.Log (f);

			if(objects["LeftArm"] != null)
			{
				objects["LeftArm"].transform.eulerAngles = new Vector3(0, f * 180, 0);
			}*/


		}
		catch( SocketException e)	
		{
			Debug.Log ("Socket Error: " + e.Message);
		}

	}
}
