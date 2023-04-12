#include "./searchDialog.hpp"
#include "./../comm/comm.hpp"


SearchDialog::SearchDialog(const wxString& title, int width, int height, std::string *animeTitle, int *mal, std::string *gogo, bool _gogo)
  : wxDialog(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(width, height)) {
  m_animeTitle = animeTitle, m_mal = mal, m_gogo = gogo;
  b_gogo = _gogo;

  wxPanel *panel = new wxPanel(this, -1);

  wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *hbox1 = new wxBoxSizer(wxHORIZONTAL);
  wxBoxSizer *hbox2 = new wxBoxSizer(wxHORIZONTAL);

  wxStaticText *template_Search = new wxStaticText(panel, -1, std::string("Search ") + ((b_gogo) ? "Gogo: " : "MAL: "));
  search = new wxTextCtrl(panel, -1, animeTitle->data(), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
  hbox1->Add(template_Search, 1, wxCENTRE); hbox1->Add(search, 3, wxCENTRE | wxLEFT, 5);

  resultListBox = new wxListBox(panel, -1);
  hbox2->Add(resultListBox, 1, wxEXPAND);

  vbox->Add(hbox1, 1, wxCENTRE | wxALL, 10);
  vbox->Add(hbox2, 5, wxCENTRE | wxEXPAND | wxALL, 10);
  panel->SetSizer(vbox);

  search->Bind(wxEVT_COMMAND_TEXT_ENTER, &SearchDialog::OnSearch, this);
  resultListBox->Bind(wxEVT_LISTBOX_DCLICK, &SearchDialog::OnSelect, this);

  if (b_gogo) OnSearch();

  Centre();
  ShowModal();

  Destroy();
}

void SearchDialog::OnSearch() {
  std::string searchValue = search->GetValue().ToStdString();

  if (b_gogo) m_animeList = searchGogo(searchValue);
  else m_animeList = searchMAL(searchValue);

  int i; resultListBox->Clear();
  for (i = 0; i < m_animeList.size(); ++i) {
    resultListBox->Append(m_animeList[i]["title"].get<std::string>());
  }
  if (resultListBox->GetCount()) resultListBox->SetSelection(0);
}
void SearchDialog::OnSearch(wxCommandEvent& event) {
  OnSearch();
}
void SearchDialog::OnSelect(wxCommandEvent& event) {
  int sel = resultListBox->GetSelection();
  if (sel != -1) {
    if (b_gogo) {
      m_gogo->assign(m_animeList[sel]["link"].get<std::string>());
      this->Close();
    } else { m_animeTitle->assign(m_animeList[sel]["title"].get<std::string>());
      SearchDialog *gogoDialog = new SearchDialog(m_title.ToStdString(), m_width, m_height, m_animeTitle, m_mal, m_gogo, 1);

      gogoDialog->Show();
      if (m_gogo->empty() == 0) {
        *m_mal = m_animeList[sel]["link"].get<int>();
        this->Close();
      }
    }
  }
}
