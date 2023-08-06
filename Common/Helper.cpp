#include "pch.h"
#include "Helper.h"
#include <string>
#include <locale>
#include <codecvt>
#include <comdef.h> 


LPCTSTR GetComErrorString(HRESULT hr)
{
	_com_error err(hr);
	LPCTSTR errMsg = err.ErrorMessage();
	return errMsg;
}

