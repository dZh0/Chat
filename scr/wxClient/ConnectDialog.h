#pragma once

#include <wx/dialog.h>

class ConnectDialog : public wxDialog
{
public:
	ConnectDialog();
private:
	wxString server	= "localhost";
	wxString port	= "1234";
	wxString name	= "Bob";
};
