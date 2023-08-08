#include "pch.h"
#include "Helper.h"
#include <comdef.h>


LPCWSTR GetComErrorString(HRESULT hr)
{
	_com_error err(hr);
	LPCWSTR errMsg = err.ErrorMessage();
	return errMsg;
}

