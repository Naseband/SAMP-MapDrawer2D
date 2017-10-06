#if defined __F_ARRAY_INCLUDED__
#endinput
#endif
#define __F_ARRAY_INCLUDED__

/*============================================================================================================================\\
||
||
||	File Array Include (v1) by NaS (c) 2017
||
||
||	This Include is not meant for improving anything.
||	It's just a simple way to get a lot of dynamic storing space, however it is pretty slow.
||
||	There are 3 predefined Data Types (modes):
||	Byte, Integer and Floating Point.
||
||	
||	Also supports saving a whole Array, and load it as it was before.
||	With this, you can easily dump whole Arrays to a file. Best suits Test Scripts,
||	or systems that aren't relying on Speed.
||
||
||	Base Functions:
||
||		Function:		CreateFileArray(size, bool:tempfile = true, mode = F_ARRAY_MODE_BYTE, init = 0, const filename[] = "")
||		Purpose:		Creates a File Array
||		Returns:		ID if successful, otherwise -1
||		Notes:			Will initialize all slots with 0 by default. Existing files will be removed.
||						If it's not a temporary file and no filename is given, the file will be located in "scriptfiles/fa/".
||						Temporary files will be located in the hard drive's root directory.
||
||
||		Function:		OpenFileArray(size, const filename[], mode = F_ARRAY_MODE_BYTE)
||		Purpose:		Opens an existing file (by its full filename includig path) as farray.
||		Returns:		ID if successful, otherwise -1
||		Notes:			Sizes do not have to match (overhead will be kept, missing space will be allocated and init. with 0).
||
||
||		Function:		DestroyFileArray(id)
||		Purpose:		Destroys a File Array (file will be kept except temporary)
||		Returns:		1 if successful, otherwise 0
||
||
||	Universal Data Get/Set:
||
||		Function:		GetFArray(id, index, mode = -1)
||		Purpose:		Get the Value of an index, any Data Type
||		Returns:		Value if successful, otherwise -1
||		Notes:			You can use this on any Array of any Data Type, 
||						however reading Int from a Byte Array will mix up the offsets (Index * Size)	
||
||		Function:		SetFArray(id, index, value, mode = -1)
||		Purpose:		Set the Value of an Index, any Data Type
||		Returns:		1 if successful, otherwise 0	
||
||
||	Data Type specific Get/Set (Macros):
||
||		Purpose:		Safe access for each Data Type
||		Returns:		See GetFArray/SetFArray
||
||		getByte(id, index);
||		setByte(id, index, value);
||
||		getInt(id, index);
||		setInt(id, index, value)
||
||		Float:getFloat(id, index);
||		setFloat(id, index, Float:value);
||
\\============================================================================================================================*/


#if !defined MAX_F_ARRAYS
	#define MAX_F_ARRAYS				10
#else
#if MAX_F_ARRAYS < 1
	#error "Define MAX_F_ARRAYS cannot be below one."
#endif
#endif

enum
{
	F_ARRAY_MODE_BYTE = 0,
	F_ARRAY_MODE_INT,
	F_ARRAY_MODE_FLOAT
};

enum E_F_ARRAY
{
	bool:_faUsed,
	File:_faHandle,
	_faSize,
	bool:_faTemp,
	_faMode
};
stock _gFArrays[MAX_F_ARRAYS][E_F_ARRAY];


// BASE

//native CreateFileArray(size, bool:tempfile = true, mode = F_ARRAY_MODE_BYTE, init = 0, const filename[] = "");
stock CreateFileArray(size, bool:tempfile = true, mode = F_ARRAY_MODE_BYTE, init = 0, const filename[] = "")
{
	if(size < 0) return -1;
	
	new id = -1;
	for(new i = 0; i < MAX_F_ARRAYS; i ++) if(!_gFArrays[i][_faUsed])
	{
	    id = i;
	    break;
	}
	
	if(id == -1) return -1;
	
	new File:handle;
	
	if(tempfile) handle = ftemp();
	else
	{
		if(filename[0] == 0)
		{
    		new fname[15];
	 		format(fname, sizeof(fname), "fa/%00005d.tmp", id);

	 		if(fexist(fname)) fremove(fname);
	 		handle = fopen(fname, io_readwrite);
		}
		else
		{
			if(fexist(filename)) fremove(filename);
			handle = fopen(filename, io_readwrite);
		}
	}
	
	if(!handle) return -1;
	
	_gFArrays[id][_faHandle] = handle;
	_gFArrays[id][_faSize] = size;
	_gFArrays[id][_faUsed] = true;
	_gFArrays[id][_faTemp] = tempfile;
	_gFArrays[id][_faMode] = mode;

	switch(mode)
	{
		case F_ARRAY_MODE_INT, F_ARRAY_MODE_FLOAT:
		{
			size *= 4;
		}
	}
	
	if(init) for(new i = 0; i < size; i ++) fputchar(handle, 0, false); // Writes all zeros
	else // This puts a character at the end to "fill" the file, if the file is already at this size it will re-write the last character (to make sure it is big enough)
	{
		fseek(handle, size, seek_start);
		new c = fgetchar(handle, 0, false);

		if(c >= 0 && c < 256) fputchar(handle, c, false);
		else fputchar(handle, 0, false);
	}
	
	return id;
}

//native OpenFileArray(size, const filename[], mode = F_ARRAY_MODE_BYTE);
stock OpenFileArray(size, const filename[], mode = F_ARRAY_MODE_BYTE)
{
	if(size < 0) return -1;
	
	new id = -1;
	for(new i = 0; i < MAX_F_ARRAYS; i ++) if(!_gFArrays[i][_faUsed])
	{
	    id = i;
	    break;
	}
	
	if(id == -1) return -1;
	
	new File:handle = fopen(filename, io_readwrite); 
	
	if(!handle) return -1;
	
	_gFArrays[id][_faHandle] = handle;
	_gFArrays[id][_faSize] = size;
	_gFArrays[id][_faUsed] = true;
	_gFArrays[id][_faTemp] = true;
	_gFArrays[id][_faMode] = mode;

	switch(mode)
	{
		case F_ARRAY_MODE_INT, F_ARRAY_MODE_FLOAT:
		{
			size *= 4;
		}
	}

	fseek(handle, size, seek_start);
	new c = fgetchar(handle, 0, false);

	if(c >= 0 && c < 256) fputchar(handle, c, false);
	else fputchar(handle, 0, false);
	
	return id;
}

//native DestroyFileArray(id);
stock DestroyFileArray(id)
{
	if(!IsValidFileArray(id)) return 0;
	
	fclose(_gFArrays[id][_faHandle]);
	_gFArrays[id][_faHandle] = File:0;
	
	_gFArrays[id][_faUsed] = false;
	
	return 1;
}

//native IsValidFileArray(id)
stock IsValidFileArray(id)
{
	if(id < 0 || id >= MAX_F_ARRAYS) return 0;

	return (_gFArrays[id][_faUsed] ? 1 : 0);
}

//native GetFileArraySize(id)
stock GetFileArraySize(id)
{
	if(!IsValidFileArray(id)) return 0;

	return _gFArrays[id][_faSize];
}

//native GetFileArrayMode(id)
stock GetFileArrayMode(id)
{
	if(!IsValidFileArray(id)) return 0;

	return _gFArrays[id][_faMode];
}

//native IsFileArrayTemporary(id)
stock IsFileArrayTemporary(id)
{
	if(!IsValidFileArray(id)) return 0;

	return (_gFArrays[id][_faTemp] ? 1 : 0);
}

//native File:GetFileArrayHandle(id)
stock File:GetFileArrayHandle(id) // Note: If a File Array exists, the Handle will always be valid and active.
{
	if(!IsValidFileArray(id)) return 0;

	return _gFArrays[id][_faHandle];
}


// DATA GET/SET

//native GetFArray(id, index, mode = -1);
stock GetFArray(id, index, mode = -1)
{
	if(!IsValidFileArray(id)) return -1;

	if(_gFArrays[id][_faMode] != mode && mode != -1) return -1;

	if(index < 0 || index >= _gFArrays[id][_faSize]) return -1;

	switch(_gFArrays[id][_faMode])
	{
		case F_ARRAY_MODE_BYTE: // 1 Byte
		{
			new File:handle = _gFArrays[id][_faHandle];

			fseek(handle, index, seek_start);

			return fgetchar(handle, 0, false);
		}
		case F_ARRAY_MODE_INT, F_ARRAY_MODE_FLOAT: // 4 Byte (Float just needs to get tagged)
		{
			new c24, c16, c8, c0, File:handle = _gFArrays[id][_faHandle];

			fseek(handle, index * 4, seek_start);

			c0 = fgetchar(handle, 0, false);
			c8 = fgetchar(handle, 0, false);
			c16 = fgetchar(handle, 0, false);
			c24 = fgetchar(handle, 0, false);

			return c24 * 16777216 + c16 * 65536 + c8 * 256 + c0;
		}
	}

	return -1;
}

//native SetFArray(id, index, value, mode = -1);
stock SetFArray(id, index, value, mode = -1)
{
	if(!IsValidFileArray(id)) return 0;
	
	if(_gFArrays[id][_faMode] != mode && mode != -1) return 0;

	switch(_gFArrays[id][_faMode])
	{
		case F_ARRAY_MODE_BYTE: // 1 Byte
		{
			if(value < 0 || value >= 256) return 0;

			new File:handle = _gFArrays[id][_faHandle];

			fseek(handle, index, seek_start);
			fputchar(handle, value, false);

			return 1;
		}
		case F_ARRAY_MODE_INT, F_ARRAY_MODE_FLOAT: // 4 Byte (Float just needs to get tagged)
		{
			new c24, c16, c8, c0, File:handle = _gFArrays[id][_faHandle];

			c24 = (value >> 24) & 0xFF;
			c16 = (value >> 16) & 0xFF;
			c8 = (value >> 8) & 0xFF;
			c0 = value & 0xFF;

			fseek(handle, index * 4, seek_start);

			fputchar(handle, c0, false);
			fputchar(handle, c8, false);
			fputchar(handle, c16, false);
			fputchar(handle, c24, false);

			return 1;
		}
	}

	return 0;
}


// BYTE GET/SET

//native getByte(id, index);
stock getByte(id, index) return GetFArray(id, index, F_ARRAY_MODE_BYTE);
//native setByte(id, index, value);
stock setByte(id, index, value) return SetFArray(id, index, value, F_ARRAY_MODE_BYTE);


// INT GET/SET

//native getInt(id, index);
stock getInt(id, index) return GetFArray(id, index, F_ARRAY_MODE_INT);
//native setInt(id, index, value);
stock setInt(id, index, value) return SetFArray(id, index, value, F_ARRAY_MODE_INT);


// FLOAT GET/SET

//native Float:getFloat(id, index);
stock Float:getFloat(id, index) return Float:GetFArray(id, index, F_ARRAY_MODE_FLOAT);
//native setFloat(id, index, Float:value);
stock setFloat(id, index, Float:value) return SetFArray(id, index, _:value, F_ARRAY_MODE_FLOAT);

// INT ARRAY GET/SET

//native getArrayInt(id, index, array[], size = sizeof array)
stock getArrayInt(id, index, array[], size = sizeof array)
{
	if(!IsValidFileArray(id)) return 0;

	if(index < 0 || index >= _gFArrays[id][_faSize]) return 0;

	for(new i = 0; i < size; i ++) array[i] = getInt(id, index + i);

	return 1;
}

//native setArrayInt(id, index, array[], size = sizeof array);
stock setArrayInt(id, index, array[], size = sizeof array)
{
	if(!IsValidFileArray(id)) return 0;

	if(index < 0 || index >= _gFArrays[id][_faSize]) return 0;

	for(new i = 0; i < size; i ++) setInt(id, index + i, array[i]);

	return 1;
}

// EOF