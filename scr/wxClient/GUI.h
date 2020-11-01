#pragma once
#include <wx/wx.h>

class ChatWindow : public wxFrame
{
public:
	ChatWindow();
	void Connect();
private:
	void OnConnect(wxCommandEvent& event);
	void OnConnected(wxCommandEvent& event);
	void OnExit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void Update(wxIdleEvent& event);
};

class ConnectDialog : public wxDialog
{
public:
	ConnectDialog();
	
	bool Validate() override;
	bool TransferDataFromWindow() override;
};