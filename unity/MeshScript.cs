using UnityEngine;
using System.Collections;

using System;
using System.Runtime.InteropServices;

public class MeshScript : MonoBehaviour {


    [DllImport("MocapPipe")]
    public static extern int pointSize();

    [DllImport("MocapPipe")]
    public static extern int vertsSize();

    [DllImport("MocapPipe")]
    public static extern int getData(float[] points, int[] verts);


    Mesh mesh;

    // Use this for initialization
    void Start () {

        gameObject.AddComponent<MeshFilter>();
        gameObject.AddComponent<MeshRenderer>();
        mesh = GetComponent<MeshFilter>().mesh;


    }

    // Update is called once per frame
    void Update()
    {

        int pointLen = pointSize() / sizeof(float);

        int vertLen = vertsSize() / sizeof(int);

        if (pointLen == 0 || vertLen == 0) return;

        //Debug.Log(pointLen);
        //Debug.Log(vertLen);

        float[] pointData = new float[pointLen];
        int[] vertData = new int[vertLen];

        getData(pointData, vertData);

        Vector3[] points = new Vector3[pointLen];


        
        for (int i=0, p=0;  i < pointLen; p++)
        {
            points[p].x = pointData[i]; i++;
            points[p].y = pointData[i]; i++;
            points[p].z = pointData[i]; i++;
        }

        mesh.Clear();
        mesh.vertices = points;
        //mesh.uv = new Vector2[] { new Vector2(0, 0), new Vector2(0, 1), new Vector2(1, 1) };
        mesh.triangles = vertData;
    }
}
/*

       NamedPipeClientStream client = new NamedPipeClientStream(".", "vrmesh", PipeDirection.In);

       BinaryReader r;

       client.Connect();

       Debug.Log(client.IsConnected);

       if (!client.IsConnected) return;

       r = new BinaryReader(client);

       int pointLen = r.ReadInt32();
       int countLen = r.ReadInt32();
       int vertsLen = r.ReadInt32();

       Debug.Log(pointLen);
       Debug.Log(countLen);
       Debug.Log(vertsLen);


       Vector3[] points = new Vector3[pointLen];
       int[] counts = new int[countLen];
       int[] verts = new int[vertsLen];

       int i;
       float x, y, z, w;
       for (i = 0; i < pointLen; i++)
       {
           x = (float)r.ReadDouble();
           y = (float)r.ReadDouble();
           z = (float)r.ReadDouble();
           w = (float)r.ReadDouble();
           points[i] = new Vector3(x, y, z);
       }

       for (i = 0; i < countLen; i++)
           counts[i] = r.ReadInt32();

       for (i = 0; i < vertsLen; i++)
           verts[i] = r.ReadInt32();


   */
