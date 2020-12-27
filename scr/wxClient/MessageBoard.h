#pragma once
#include <wx/scrolwin.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>

class MessageBoard : public wxScrolledWindow
{
public:
	MessageBoard(
		wxWindow* parent,
		wxWindowID winid = wxID_ANY,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxScrolledWindowStyle,
		const wxString& name = wxASCII_STR(wxPanelNameStr)
	);
	virtual void AddMessage(const wxString& messageContent, const wxString& senderName); //TODO: Remove
	void OnMessage(wxCommandEvent &event);
};

class TextMessage : public wxPanel
{
public:
	TextMessage():wxPanel(){};
	bool Create(wxWindow* parent, wxWindowID winid, const wxString& senderName, const wxString& content);
};