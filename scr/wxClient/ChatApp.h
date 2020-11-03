#pragma once
#include <wx/wx.h>
//#include "Client.h"



class ChatApp : public wxApp
{
public:
	virtual bool OnInit();
	virtual void Connect();
private:
	bool isConnectionSet = false;
};
wxDECLARE_APP(ChatApp);

wxDECLARE_EVENT(EVT_NETWORK, wxCommandEvent);

constexpr int MESSAGE_RECIEVED_ID = 10000;
class UpdateThread : public wxThread
{
public:
	UpdateThread(wxFrame* parent);
	virtual ExitCode Entry();
private:
	wxFrame* m_parent;
};
