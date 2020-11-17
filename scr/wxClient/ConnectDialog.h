#pragma once

#include <string>
#include <wx/dialog.h>

class ConnectDialog : public wxDialog
{
public:
	ConnectDialog(wxString &serverIp, wxString &serverPort, wxString &userName);
};
