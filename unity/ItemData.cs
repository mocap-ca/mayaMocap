using UnityEngine;
using System;
using System.Collections.Generic;

namespace MocapItem
{
    // A BaseItem is the baseclass for all items of which joints, markers or other primitives 
    // are subclassed from.  It represents the smallest
    // possible granularity of data for which other datatypes can subclass and make lists
    // from.  The GameObject "obj" may exist in the scene (e.g. as a joint) or it may be
    // created during runtime (e.g. a box for a marker).


    public abstract class BaseItem
    {

        public enum Type { SEG_JOINT, SEG_MARKER };

        public BaseItem(string _name) { name = _name; }

        public abstract Type getType();
        public abstract void UpdateScene();
        public void setObject(GameObject o) { obj = o; }
        public string name;
        protected GameObject obj;
    };


    public class Joint : BaseItem
    {
        public Joint(string name, float[] _tr, float[] _ro) : base(name)
        {
            tr = _tr;
            ro = _ro;
        }

        public override Type getType() { return Type.SEG_JOINT; }

        public void Update(Joint other)
        {
            if (this.name != other.name) throw new Exception("Attempt to update joint with different name");
            this.tr = other.tr;
            this.ro = other.ro;
        }


        public override void UpdateScene()
        {
            if (obj == null) return;
            obj.transform.localPosition = new Vector3(tr[0], tr[1], tr[2]);
            obj.transform.localRotation = new Quaternion(ro[0], ro[1], ro[2], ro[3]) * rotationOffset;
        }

        public float[] tr;
        public float[] ro;
        private Quaternion rotationOffset;
    };

    public class Marker : BaseItem
    {
        public Marker(string name, float[] _tr) : base(name)
        {
            tr = _tr;
        }

        public void create(Transform parent, int markerSize)
        {
            cube = GameObject.CreatePrimitive(PrimitiveType.Cube);
            cube.transform.parent = parent;//markerGroup.transform;
            cube.name = "marker_" + name;
            cube.transform.localScale = new Vector3(markerSize, markerSize, markerSize);
            cube.transform.localPosition = new Vector3(tr[0], tr[1], tr[2]);
            obj = cube.gameObject;
        }

        public void Update(Marker other)
        {
            if (name != other.name) throw new Exception("Attempt to update marker with different name");
            tr = other.tr;
        }


        public override void UpdateScene()
        {
            if (obj == null) return;
            obj.transform.localPosition = new Vector3(tr[0], tr[1], tr[2]);
        }

        public override Type getType() { return Type.SEG_MARKER; }
        public float[] tr;
        GameObject cube;


    };

    // A subject has a list of markers and a list of joints
    public class Subject
    {
        public Subject(string _name)
        {
            name = _name;
            joints = new List<Joint>();
            markers = new List<Marker>();
        }
        public void UpdateScene()
        {
            foreach (Joint j in joints) j.UpdateScene();
            foreach (Marker m in markers) m.UpdateScene();
        }
        public void Update(Subject addSubject)
        {
            List<Joint> delJoints = new List<Joint>(joints);
            List<Marker> delMarkers = new List<Marker>(markers);

            foreach (Joint iAddJoint in addSubject.joints)
            {
                foreach (Joint iCurrentJoint in joints)
                {
                    if (iAddJoint.name == iCurrentJoint.name)
                    {
                        delJoints.Remove(iCurrentJoint); // don't delete it
                        iCurrentJoint.Update(iAddJoint);
                    }
                }
            }
            foreach (Marker iAddMarker in addSubject.markers)
            {
                foreach (Marker iCurrentMarker in markers)
                {
                    if (iAddMarker.name == iCurrentMarker.name)
                    {
                        delMarkers.Remove(iCurrentMarker); // don't delete it
                        iCurrentMarker.Update(iAddMarker);
                    }
                }
            }
            foreach (Joint j in delJoints) joints.Remove(j);
            foreach (Marker m in delMarkers) markers.Remove(m);
        }

        public string Info()
        {
            return "\t" + name + ": " + joints.Count + " joints, " + markers.Count + " markers.\n";
        }
        public void AddJoint(Joint j) { joints.Add(j); }
        public void AddMarker(Marker m) { markers.Add(m); }

        public string name;
        public List<Joint> joints;
        public List<Marker> markers;
    };

    public class SubjectList
    {
        public List<Subject> subjects = new List<Subject>();
        public void Update(Subject addSubject)
        {
            bool found = false;
            foreach (Subject i in subjects)
            {
                if (i.name == addSubject.name)
                {
                    i.Update(addSubject);
                    found = true;
                    break;
                }
            }
            if (!found) subjects.Add(addSubject);
        }
        public string Info()
        {
            string r = "Subjects: " + subjects.Count + "\n";
            foreach (Subject s in subjects) r += s.Info();
            return r;
        }
        public void Clear()
        {
            subjects = new List<Subject>();
        }
    }

    public class InfoException : Exception
    {
        public InfoException(string s) { info = s; }
        public string GetInfo() { return info; }
        private string info;
    }
} // namespace Vive