#define TX_COMPILED

template <typename DataType>
DynamicStack <DataType>::DynamicStack (size_t size /* = 0 */, const char * label /*= "unnamed"*/) :

	bound1_ (CanaryValue),

	data_ (nullptr),

	bound2_ (CanaryValue),

	size_ (0),

	length_ (0),

	label_ (label),

	hash_ (0),

	destructed_ (0)

{

	TRACE ("Constructing stack...");

	data_ = new char [sizeof (long) * 2];

	* (long*) (data_)                 = CanaryValue;
	* (long*) (data_ + sizeof (long)) = CanaryValue;

	UpdateHash ();

	resize (size);

	CHECK;

}

template <typename DataType>
DynamicStack <DataType>::~DynamicStack ()

{

	TRACE ("Destructing stack...");

	CHECK;

	delete[] (data_);

	data_ = nullptr;

	destructed_ = Defecated;

}

template <typename DataType>
void DynamicStack <DataType>::resize (size_t size)

{

	CHECK;

	TRACE ("Resizing from %zu to %zu...", size_, size);

	if (size == size_) 
		
	{

		TRACE ("Canceled: size == new size.\n");

		return;

	}

    char * new_data = new char [size * sizeof (DataType) + 2 * sizeof (long)];

	* (long*) (new_data)                                            = CanaryValue;
	* (long*) (new_data + size * sizeof (DataType) + sizeof (long)) = CanaryValue;

	for (size_t i = 0; i < size; i ++)

		if (i >= size_) Free (i, new_data);

		else *getData (new_data, i) = *getData (data_, i);

	delete[] (data_);
	data_ = new_data;

	size_ = size;

	if (length_ > size_) length_ = size_;

	UpdateHash ();

	TRACE ("Resized: new data size is %d.", sizeof (DataType) * size_ + sizeof (long) * 2);

	CHECK;

}

template <typename DataType>
void DynamicStack <DataType>::push (DataType value)

{

	CHECK

	TRACE ("Pushing 0x%X to [%zu]", value, length_);

	if (length_ >= size_) resize ((size_ + 1) * 2);

	set (length_, value);

	length_ ++;

	UpdateHash ();

	CHECK

}

template <typename DataType>
DataType DynamicStack <DataType>::pop ()

{

	CHECK

	length_ --;

	DataType value = top ();

	TRACE ("Popping 0x%X from [%zu]", value, length_);

	Free (length_);

	UpdateHash ();

	CHECK

	return value;

}

template <typename DataType>
inline void DynamicStack <DataType>::set (size_t index, DataType value)

{

	* (DataType*) (data_ + index * sizeof (DataType) + sizeof (long)) = value;

}

template <typename DataType>
inline DataType DynamicStack <DataType>::get (size_t index)

{

	return * (getData (data_, index));

}

template <typename DataType>
inline DataType DynamicStack <DataType>::top ()

{

	return get (length_);

}

template <typename DataType>
inline DataType * DynamicStack <DataType>::getData (char * data /*= nullptr*/, size_t index /*= 0*/)

{

	if (!data) data = data_;

	return (DataType*) (data + sizeof (CanaryValue) + index * sizeof (DataType));

}

template <typename DataType>
inline void DynamicStack <DataType>::Free (size_t index, char * data /*= nullptr*/)

{

	TRACE ("Freeing [%zu]", index);

	char * ptr = (char*) getData (data, index);

	for (size_t i = 0; i < sizeof (DataType) ; i++)

		ptr[i] = FreeValue;

}

template <typename DataType>
inline bool DynamicStack <DataType>::isFree (size_t index, char * data /*= nullptr*/)

{

	char * ptr = (char*) getData (data, index);

	for (size_t i = 0; i < sizeof (DataType); i++)

		if (* (ptr + i) != FreeValue) return false;

	return true;

}

template <typename DataType>
int DynamicStack <DataType>::OK ()

{

	if (length_ > size_)                     return StackErrorSize;

	if (bound1_ != CanaryValue)              return StackErrorBound1Broken;
	if (bound2_ != CanaryValue)              return StackErrorBound2Broken;

	long long hash = hash_;

	UpdateHash ();

	if (hash != hash_)                       return StackErrorWrongHash;

	//for (size_t n = length_ + 1; n < size_; n++) 
	//
	//	if (get () [n] != FreeValue)         return StackErrorValuePositionViolation;

	if (destructed_ == Defecated)            return StackErrorDestructed;
	
	if (* (long*) (data_) != CanaryValue)    return StackErrorDynamicBound1Broken;

	if (* (long*) (data_ + size_ * sizeof (DataType) + sizeof (long)) != CanaryValue)    
		
											 return StackErrorDynamicBound2Broken;

	if (length_ == SIZE_MAX)                 return StackErrorWrongLength;

	return -1;

}

long long HashOf (const void * ptr, size_t size)

{

	long long hash = 0;

	for (size_t i = 0; i < size; i++)

	{
	
		hash += *((char *)ptr + i);

	}

	return hash;

}

template <typename DataType>
void DynamicStack <DataType>::print ()

{

	CHECK;

	$sC

	printf ("|-===========Stack===========-|\n"
		    "name:  '%s', \n"
			"type:   %s,  \n"
		    "size:   %zu, \n"
	        "length: %zu, \n"
		    "bytes:  %zu, \n"
		    "hash:   %lld,\n\n"
	        "Contains:\n", label_, typeid (DataType).name (), size_, length_, size_ * sizeof (DataType) + sizeof (long) * 2, hash_);

	size_t n = 0;

	for (size_t i = 0; i < size_ * sizeof (DataType) + 2 * sizeof (CanaryValue); i)

	{

		printf ("[%zu] = ", n);

	
		if      (* (long*) (data_ + i) == CanaryValue) 
			
			printf ("[Canary] (0x%X)\n", * (long*) (data_ + i)),   i += sizeof (CanaryValue);

		else if (isFree ((i - sizeof (CanaryValue)) / sizeof (DataType))) 
			
			printf ("[Free] (0x%X)\n", * (DataType*) (data_ + i)), i += sizeof (DataType);

		else 
			
			printf ("0x%04X\n", *(data_ + i)), i += sizeof (DataType);

		n ++;

	}

	printf ("|-===========Stack===========-|\n");

}

template <typename DataType>
void DynamicStack <DataType>::printBytes ()

{

	$sc;

	printf ("Stack '%s' bytes:\n", label_);

	for (size_t i = 0; i < getBytes (); i++)

		printf ("%X ", data_[i]);

	printf ("\n");

}

const char * strDynamicStackError (int err)

{

	#define STR_ERROR(err, descr) case err: return #err ": " descr;

	switch (err)

	{
	
		STR_ERROR (StackErrorSize,                   "Size error");
		STR_ERROR (StackErrorBound1Broken,           "Left canarry broken");
		STR_ERROR (StackErrorBound2Broken,           "Right cannary broken");
		STR_ERROR (StackErrorWrongHash,              "Wrong hash");
		STR_ERROR (StackErrorValuePositionViolation, "Value position violation");
		STR_ERROR (StackErrorDestructed,             "Stack is already destructed");
		STR_ERROR (StackErrorDynamicBound1Broken,    "Left iternal stack cannary broken");
		STR_ERROR (StackErrorDynamicBound2Broken,    "Right iternal stack cannary broken");
		STR_ERROR (StackErrorWrongLength,            "Length peaked (Maybe you trying to pop () with 0 length)");

	}

	return "Unknown error";

}

template <typename unit>
void push_pop_test (unit value, size_t n)

{

	bool ok = true;

	printf ("Dynamic stack test (%zu, %zu): ", value, n);

	DynamicStack <unit> stack (n);

	for (size_t i = 0; i < n; i++)

		stack.push (value);

	if (* (long*) (stack.data_)                                               != CanaryValue) ok = false;
	if (* (long*) (stack.data_ + stack.size_ * sizeof (unit) + sizeof (long)) != CanaryValue) ok = false; 

	for (size_t i = 0; i < n; i++)

		if (stack.pop () != value) ok = false;

	if (ok) { $sG printf ("Ok\n"); }

	else   
		
	{

		$sR

		printf ("Failed: Expected\n");

		printf ("{"
				"\n[0] = [Canary]\n");

		for (size_t i = 0; i < n; i++) printf ("[%zu] = %zu\n", i, value);

		printf ("[%zu] = [Canary]\n"
			    "},\n"
			    "Got:\n" 
			    "{\n", n + 1);

		for (size_t i = 0; i < stack.size_ + 2; i++)

		{

			printf ("[%zu] = ", i);
	
			if      (stack.stack_[i] == CanaryValue) printf ("[Canary]\n");
			else if (stack.stack_[i] ==   FreeValue) printf ("[Free]\n");
			else if (stack.stack_[i] ==   Defecated) printf ("[Defecated!]\n");

			else txPrintf ("%a\n", stack.stack_[i]);

		}

		printf ("}.\n");

	}

}