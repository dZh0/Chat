#include <wx/wx.h>
#include "ChatApp.h"
#include "ConnectDialog.h"

ConnectDialog::ConnectDialog(wxWindow* parent, wxString& serverIp, wxString& serverPort, wxString& userName)
	: wxDialog(parent, wxID_ANY, "Connect to", wxDefaultPosition, wxDefaultSize, wxCAPTION | wxCLOSE_BOX, "ConnectDialog")
{
	wxPanel* panel = new wxPanel(this, -1);
	wxFlexGridSizer* grid = new wxFlexGridSizer(3, 2, 9, 10);
	grid->Add(new wxStaticText(panel, wxID_ANY, "Server IP:"));
	grid->Add(new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0, wxTextValidator(wxFILTER_EMPTY, &serverIp)), 1, wxEXPAND);
	grid->Add(new wxStaticText(panel, wxID_ANY, "Server Socket:"));
	grid->Add(new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0, wxTextValidator(wxFILTER_EMPTY | wxFILTER_DIGITS, &serverPort)), 1, wxEXPAND);
	grid->Add(new wxStaticText(panel, wxID_ANY, "User name:"), 1, wxEXPAND);
	grid->Add(new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0, wxTextValidator(wxFILTER_EMPTY, &userName)), 1, wxEXPAND);
	grid->AddGrowableCol(1, 1);

	wxBoxSizer* controls = new wxBoxSizer(wxHORIZONTAL);
	wxButton* okButton = new wxButton(panel, wxID_OK, "Connect");
	okButton->SetDefault();
	controls->Add(okButton, 0);
	controls->Add(new wxButton(panel, wxID_CANCEL, "Cancel"), 0, wxLEFT, 5);

	wxBoxSizer* contentBox = new wxBoxSizer(wxVERTICAL);
	contentBox->Add(grid, 1, wxALL | wxEXPAND, 15);
	contentBox->Add(controls, 0, wxALIGN_CENTER | wxTOP | wxBOTTOM, 10);

	panel->SetSizerAndFit(contentBox);
	Fit();
}
