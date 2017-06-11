using UnityEngine;
using System.Collections;

using System;
using System.Runtime.InteropServices;

public class MocapPipe : MonoBehaviour {

    public GameObject transformObject;

    [DllImport("MocapPipe")]
    public static extern void AddPosition(String name, float tx, float ty, float tz, float rx, float ry, float rz, float rw);

    [DllImport("MocapPipe")]
    public static extern bool Send();

    [DllImport("MocapPipe")]
    public static extern void Clear();

    [DllImport("MocapPipe")]
    public static extern void Close();
	
    [DllImport("MocapPipe")]
    public static extern char[] PointInfo();
	

	public string PointString;

    // Use this for initialization
    void Start () {
	
	}
	
	// Update is called once per frame
	void Update () {

        Clear();


        Transform t = transformObject.transform;
        float tx = t.position.x;
        float ty = t.position.y;
        float tz = t.position.z;
        float rx = t.rotation.x;
        float ry = t.rotation.y;
        float rz = t.rotation.z;
        float rw = t.rotation.w;

        AddPosition(transformObject.name, tx, ty, tz, rx, ry, rz, rw);
        Send();
		
		char[] p = PointInfo();
		PointString = new string(p);




    }
}
