using System.Collections;
using System.Collections.Generic;
using System;
using System.Text;
using UnityEngine;

public class MocapItem  
{
	public string name;

	public MocapItem() {}

	public MocapItem(string name_) 
	{
		name = name_ ;
	}
}

public class MocapSegment : MocapItem
{
	public float tx,ty, tz;
	public float rx, ry, rz, rw;

	public MocapSegment() 
	{
		tx = ty = tz = rx = ry = rz = rw = 0.0f;
	}

	public MocapSegment( string name_, float tx_, float ty_, float tz_,
                       float rx_, float ry_, float rz_, float rw_ )
	: base(name_)
	{
		tx = tx_;
		ty = ty_;
		tz = tz_;
		rx = rx_;
		ry = ry_;
		rz = rz_;
		rw = rw_;
	}

	public Quaternion GetOrientation()
	{
		return new Quaternion (rx, ry, rz, rw);
	}

	public Vector3 GetTranslation( float scale)
	{
		return new Vector3 (tx*scale, ty*scale, tz*scale);
	}
}

public class MocapMarker : MocapItem
{
	public float tx, ty, tz;

	public MocapMarker(  string name_, float tx_, float ty_, float tz_ )
		: base (name_)
	{
		tx = tx_;
		ty = ty_;
		tz = tz_;
	}
}

public class Parser
{
	public List<MocapSegment> segments;

	public Parser() { segments = new List<MocapSegment>(); }

	public int parseItems( Byte[] buffer )
	{
		segments.Clear();
	
		if( buffer.Length < 3) return 0;
	
		// The first character should be 42, the next should be the number if entries.
		if(buffer[0] != 42) throw new Exception("Invalid header");

		byte count = buffer[1];
	
		if(count == 0) return 0;
	
		int ptr = 2;
	
	
		for (int i = 0; i < count && ptr < buffer.Length; i++)
		{
		
			byte id = buffer[ptr++];
			if (id != 43 && id != 44 && id != 45) throw new Exception("Invalid packet entry");
			
			byte namelen = buffer[ptr++];  
			byte datalen = buffer[ptr++];  

			int name = ptr; 
			ptr += namelen;
			int data = ptr;
			ptr += datalen;
			
			if (ptr > buffer.Length) throw new Exception("Parser buffer overrun");
			
			if( id == 43 )
			{
				MocapSegment segment = new MocapSegment();
				
				// Name
				for(int q = 0; q < namelen; q++) 
				{
					segment.name += (char)buffer[ name + q ];
				}


				int n = 0;
				int c = 0;
				string temp = "";

				while(n<7)
				{
					byte ch = buffer[ data + c ];
					c++;
					if ( ch != 0 ) 
					{
						temp += (char)ch;
						continue;
					}

					if(n==0) float.TryParse ( temp, out segment.tx );
					if(n==1) float.TryParse ( temp, out segment.ty );
					if(n==2) float.TryParse ( temp, out segment.tz );
					if(n==3) float.TryParse ( temp, out segment.rx );
					if(n==4) float.TryParse ( temp, out segment.ry );
					if(n==5) float.TryParse ( temp, out segment.rz );
					if(n==6) float.TryParse ( temp, out segment.rw );

					temp = "";


					n++;



				}
				
				segments.Add(segment);
				
			}
			
			/*if( id == 44 )
			{
				Marker marker = new Marker();
				
				// Name
				memcpy(marker->name, name, namelen);
				marker->name[namelen] = 0;
				
				float      *fptr = &marker->tx;
				size_t      dptr = 0;
				
				for (int j = 0; j < 3 && data + dptr < buffer + len; j++)
				{
					fptr[j] = atof(data + dptr);
					dptr += strlen(data + dptr) + 1;
				}
				
				items.Add(marker);
				
			}*/
			
			/*if (id == 45)
			{
				char tmp[64];
				memcpy(tmp, data, datalen);
				tmp[datalen] = 0;
				
				float f1 = 0.0f;
				float f2 = 0.0f;
				int ret = sscanf(tmp, "%f %f", &f1, &f2);
				
				if (ret == 2)
				{
					Marker *marker = new Marker();
					
					// Name
					memcpy(marker->name, name, namelen);
					marker->name[namelen] = 0;
					
					marker->tx = f1;
					marker->ty = f2;
					marker->tz = 0.0f;
					
					items->push_back(marker);
					
				}
			}*/
		}
		
		return ptr;
	}
}
