using UnityEngine;
using System.Collections;
using System.Collections.Generic;

using System;
using System.Runtime.InteropServices;

public class MocapTest : MonoBehaviour {

    [DllImport("UnityMocap", CallingConvention = CallingConvention.Cdecl)]
    public static extern void mocapBind(int val );

    [DllImport("UnityMocap", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool mocapBound();

    [DllImport("UnityMocap", CallingConvention = CallingConvention.Cdecl)]
    public static extern int markers();

    [DllImport("UnityMocap", CallingConvention = CallingConvention.Cdecl)]
    public static extern int segments();

    [DllImport("UnityMocap", CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr ThreadInfo();

    [DllImport("UnityMocap", CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr getSegment(int id, ref float tx, ref float ty, ref float tz, ref float rx, ref float ry, ref float rz, ref float rw);

    public string info;

    public int markerCount;

    public int segmentCount;

    List<GameObject> mocapTagged = new List<GameObject>();

    public Vector3 markerSize = new Vector3(0.05f, 0.3f, 0.05f);

    

    // Use this for initialization
    void Start () {
        mocapBind(9119);

        GameObject[] objs = GameObject.FindGameObjectsWithTag("Mocap");
        for (int i = 0; i < objs.Length; i++)
            mocapTagged.Add(objs[i]);

    }
	
	// Update is called once per frame
	void Update () {
        markerCount = markers();
        segmentCount = segments();

        info = Marshal.PtrToStringAnsi(ThreadInfo());

        float tx = 0.0f, ty = 0.0f, tz = 0.0f, rx = 0.0f, ry = 0.0f, rz = 0.0f, rw = 0.0f;

        for (int segment = 0; segment < segmentCount; segment++)
        {            
            String name = Marshal.PtrToStringAnsi(getSegment(segment, ref tx, ref ty, ref tz, ref rx, ref ry, ref rz, ref rw));
            //Debug.Log(name);

            bool found = false;
            foreach( GameObject obj in mocapTagged)
            {
                if (name == obj.name)
                {
                    obj.transform.localPosition = new Vector3(tx, ty, tz);
                    obj.transform.localRotation = new Quaternion(rx, ry, rz, rw);
                    found = true;
                    break;
                }
            }
            if(!found)
            {
                GameObject cube = GameObject.CreatePrimitive(PrimitiveType.Cube);
                cube.name = name;
                cube.transform.localScale = markerSize;
                cube.transform.localPosition = new Vector3(tx, ty, tz);
                cube.transform.localRotation = new Quaternion(rx, ry, rz, rw);
                mocapTagged.Add(cube);
            }
        }
	}
}
