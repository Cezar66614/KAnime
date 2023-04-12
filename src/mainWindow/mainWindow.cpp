#include "./mainWindow.hpp"
#include "./../searchDialog/searchDialog.hpp"
#include "./../comm/comm.hpp"

MainWindow::MainWindow(const wxString& title, int width, int height, json &profile)
  : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(width, height)) { m_profile = profile;

  menubar = new wxMenuBar;
  file = new wxMenu;
  file->Append(ID_IMPORT_DIR_MENU, wxT("&Import Folder"), wxT("Adds animes from a directory into the currently in-use profile"));
  file->Append(ID_ADD_MANUAL, wxT("&Add anime manually"), wxT("The user types the MAL and Gogo code themselves"));
  menubar->Append(file, wxT("&File"));

  settings = new wxMenu;
  settings->Append(ID_SET_DEFAULT_PATH, wxT("&Set Default Path"), wxT("Sets the default download directory path"));
  menubar->Append(settings, wxT("&Settings"));

  SetMenuBar(menubar);

  wxPanel *panel = new wxPanel(this, -1);
  listPanel = new wxPanel(panel);
  buttonPanel = new wxPanel(listPanel);
  previewPanel = new PreviewPanel(panel);

  wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
  wxBoxSizer *vbox1 = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *vbox2 = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *vbox11 = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *vbox111 = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *hbox111 = new wxBoxSizer(wxHORIZONTAL);
  wxBoxSizer *hbox112 = new wxBoxSizer(wxHORIZONTAL);

  animeListBox = new wxListBox(listPanel, -1);

  refreshButton = new wxButton(buttonPanel, ID_REFRESH_BUTTON, wxT("Refresh List"));
  addButton = new wxButton(buttonPanel, ID_ADD_BUTTON, wxT("Add Anime"));
  removeButton = new wxButton(buttonPanel, ID_REMOVE_BUTTON, wxT("Remove Anime"));
  upButton = new wxButton(buttonPanel, ID_UP_BUTTON, wxT("↑"));
  downButton = new wxButton(buttonPanel, ID_DOWN_BUTTON, wxT("↓"));
  checkButton = new wxButton(buttonPanel, ID_CHECK_ALL_BUTTON, wxT("Check Ep"));

  hbox111->Add(addButton); hbox111->Add(refreshButton); hbox111->Add(upButton);
  hbox112->Add(removeButton); hbox112->Add(checkButton); hbox112->Add(downButton);

  vbox111->Add(hbox111, 1, wxALIGN_CENTER | wxLEFT, 5); vbox111->Add(hbox112, 1, wxALIGN_CENTER);
  buttonPanel->SetSizer(vbox111);

  vbox11->Add(animeListBox, 100, wxEXPAND);
  vbox11->Add(buttonPanel, 1, wxALIGN_CENTER | wxTOP, 10);

  listPanel->SetSizer(vbox11);

  vbox1->Add(listPanel, 1, wxEXPAND);
  vbox2->Add(previewPanel, 1, wxEXPAND);

  hbox->Add(vbox1, 1, wxALIGN_LEFT | wxALL, 10);
  hbox->Add(vbox2, 100, wxEXPAND | wxALL, 10);

  vbox->Add(hbox, 1, wxEXPAND);
  panel->SetSizer(vbox);

  loadProfile();

  Bind(wxEVT_SIZE, &MainWindow::OnResize, this);

  Bind(wxEVT_BUTTON, &MainWindow::OnRefresh, this, ID_REFRESH_BUTTON);
  Bind(wxEVT_BUTTON, &MainWindow::OnAdd, this, ID_ADD_BUTTON);
  Bind(wxEVT_BUTTON, &MainWindow::OnRemove, this, ID_REMOVE_BUTTON);
  Bind(wxEVT_BUTTON, &MainWindow::OnUp, this, ID_UP_BUTTON);
  Bind(wxEVT_BUTTON, &MainWindow::OnDown, this, ID_DOWN_BUTTON);
  Bind(wxEVT_BUTTON, &MainWindow::OnCheck, this, ID_CHECK_ALL_BUTTON);

  Bind(wxEVT_MENU, &MainWindow::SetDefaultPath, this, ID_SET_DEFAULT_PATH);
  Bind(wxEVT_MENU, &MainWindow::ImportDir, this, ID_IMPORT_DIR_MENU);
  Bind(wxEVT_MENU, &MainWindow::OnAddManual, this, ID_ADD_MANUAL);

  animeListBox->Bind(wxEVT_LISTBOX_DCLICK, &MainWindow::LoadPreview, this);

  Centre();
}

void MainWindow::OnResize(wxSizeEvent& event) {
  listPanel->SetSize(wxSize(listPanel->m_width, this->m_height));
  animeListBox->SetSize(listPanel->GetSize());
  animeListBox->SetMaxSize(wxSize(310, this->m_height));

  Layout(); event.Skip();
}

void MainWindow::loadProfile() {
  try {
    animeListBox->Clear();
    loadCategory("Airing");
    loadCategory("Finished");
  } catch (...) { std::fprintf(stderr, "Error at listing profile file!\n"); }
}
void MainWindow::loadCategory(const char *categ) { int i, siz = 0;
  try {
    siz = m_profile["anime"][categ].size();
    if (siz) animeListBox->Append(categ);
    for (i = 0; i < siz; ++i) animeListBox->Append(m_profile["anime"][categ][i]["name"].get<std::string>());
  } catch (...) { std::fprintf(stderr, "Unable to load category: %s!\n", categ); }
}
void MainWindow::readProfile() { readJson("profile.kanime", m_profile); }
void MainWindow::writeProfile() { writeJson("profile.kanime", m_profile); }

void MainWindow::OnRefresh(wxCommandEvent &event) { readProfile(); loadProfile(); }
void MainWindow::OnAdd(wxCommandEvent &event) {
  try {
    std::string name, link_mal, link_gogo; int mal;

    SearchDialog *searchDialog = new SearchDialog(wxT("Search Dialog"), 400, 300, &name, &mal, &link_gogo, 0);
    searchDialog->Show(); link_mal = std::to_string(mal);
    delete searchDialog;

    if (name.empty() == 0 && animeListBox->FindString(name) == -1) {
      wxDirDialog *folderDialog = new wxDirDialog(this, name + " Anime Download Directory", (m_profile["settings"]["defaultPath"].empty()) ? std::string() : m_profile["settings"]["defaultPath"].get<std::string>(), wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
    
      if (folderDialog->ShowModal() == wxID_OK) {
        wxString path = folderDialog->GetPath();
        if (!path.empty()) {
          // Get details from MAL
          bool status; std::string image_link;
          int total_ep = getAnimeDetails(link_mal, status, image_link);

          char category[12];
          if (status) std::strcpy(category, "Airing"); else std::strcpy(category, "Finished");

          int i = m_profile["anime"][category].size();
          m_profile["anime"][category][i]["name"] = name;
          m_profile["anime"][category][i]["path"] = path;
          m_profile["anime"][category][i]["id"] = i;

          m_active["name"] = name;
          m_active["link_mal"] = link_mal;
          m_active["link_gogo"] = link_gogo;
          m_active["diskEp"] = getLastDownEp(path.ToStdString());
          m_active["airedEp"] = getLastGogoEp(link_gogo);
          m_active["totalEp"] = total_ep;
          m_active["link_img"] = image_link;
          writeJson(std::string(path.ToStdString() + "/data.json").c_str(), m_active);
          urlImageToDisk(image_link, path.ToStdString());

          writeProfile();
          loadProfile();
        }
      } delete folderDialog;
    }
  } catch (...) { std::fprintf(stderr, "Unable to add anime!\n"); }
}
void MainWindow::OnAddManual(wxCommandEvent &event) {
  try {
    wxString name = wxGetTextFromUser(wxT("Anime Name"));
    if (name.empty() == 0 && animeListBox->FindString(name) == -1) {
      std::string link_mal = wxGetTextFromUser(wxT("MAL Link")).ToStdString(); size_t pos, i;
      if (link_mal.empty() == 0) { pos = link_mal.find("anime/");
        if (pos != std::string::npos) { for (i = 0, pos += 6; pos < link_mal.size() && link_mal[pos] != '/'; ++pos, ++i) link_mal[i] = link_mal[pos]; link_mal.resize(i); }
        else return;
      } else return;

      std::string link_gogo = wxGetTextFromUser(wxT("Gogo Link")).ToStdString();
      if (link_gogo.empty() == 0) { pos = link_gogo.find("category/");
        if (pos != std::string::npos) { for (i = 0, pos += 9; pos < link_gogo.size() && link_gogo[pos] != '/'; ++pos, ++i) link_gogo[i] = link_gogo[pos]; link_gogo.resize(i); }
        else return;
      } else return;

      wxDirDialog *folderDialog = new wxDirDialog(this, name + " Anime Download Directory", (m_profile["settings"]["defaultPath"].empty()) ? std::string() : m_profile["settings"]["defaultPath"].get<std::string>(), wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
      if (folderDialog->ShowModal() == wxID_OK) {
        wxString path = folderDialog->GetPath();
        if (!path.empty()) {
          // Get details from MAL
          bool status; std::string image_link;
          int total_ep = getAnimeDetails(link_mal, status, image_link);

          char category[12];
          if (status) std::strcpy(category, "Airing"); else std::strcpy(category, "Finished");

          int i = m_profile["anime"][category].size();
          m_profile["anime"][category][i]["name"] = name;
          m_profile["anime"][category][i]["path"] = path;
          m_profile["anime"][category][i]["id"] = i;

          m_active["name"] = name;
          m_active["link_mal"] = link_mal;
          m_active["link_gogo"] = link_gogo;
          m_active["diskEp"] = getLastDownEp(path.ToStdString());
          m_active["airedEp"] = getLastGogoEp(link_gogo);
          m_active["totalEp"] = total_ep;
          m_active["link_img"] = image_link;
          writeJson(std::string(path.ToStdString() + "/data.json").c_str(), m_active);
          urlImageToDisk(image_link, path.ToStdString());

          writeProfile();
          loadProfile();
        }
      } delete folderDialog;
    }
  } catch (...) { std::fprintf(stderr, "Unable to add anime!\n"); }
}
void MainWindow::OnRemove(wxCommandEvent &event) {
  try {
    int sel = animeListBox->GetSelection();

    int siz = m_profile["anime"]["Airing"].size(); bool categ = 0;
    if (siz) {
      if (sel <= siz) categ = 1;
      else sel -= siz + 1;
    } sel -= 1;

    if (sel >= 0) {
      m_profile["anime"][(categ) ? "Airing" : "Finished"].erase(sel);
      siz = m_profile["anime"][(categ) ? "Airing" : "Finished"].size();
      int i;
      for (i = sel; i < siz; ++i) m_profile["anime"][(categ) ? "Airing" : "Finished"][i]["id"] = i;

      writeProfile();
      loadProfile();
    }
  } catch (...) { std::fprintf(stderr, "Unable to remove anime!\n"); }
}
void MainWindow::OnUp(wxCommandEvent &event) {
  try {
    int sel = animeListBox->GetSelection(), c_sel = sel;

    int sizAir = m_profile["anime"]["Airing"].size(); bool categ = 0;
    if (sizAir) {
      if (sel <= sizAir) categ = 1;
      else sel -= sizAir + 1;
    } sel -= 1;

    if (sel > 0) {
      std::string aux = m_profile["anime"][(categ) ? "Airing" : "Finished"][sel].dump(2);
      m_profile["anime"][(categ) ? "Airing" : "Finished"][sel] = m_profile["anime"][(categ) ? "Airing" : "Finished"][sel-1];
      m_profile["anime"][(categ) ? "Airing" : "Finished"][sel-1] = json::parse(aux);

      m_profile["anime"][(categ) ? "Airing" : "Finished"][sel-1]["id"] = sel-1;
      m_profile["anime"][(categ) ? "Airing" : "Finished"][sel]["id"] = sel;

      writeProfile();
      loadProfile();

      animeListBox->SetSelection(c_sel-1);
    }
  } catch (...) { std::fprintf(stderr, "Unable to move anime UP!\n"); }
}
void MainWindow::OnDown(wxCommandEvent &event) {
  try {
    int sel = animeListBox->GetSelection(), c_sel = sel;

    int sizAir = m_profile["anime"]["Airing"].size(); bool categ = 0;
    if (sizAir) {
      if (sel <= sizAir) categ = 1;
      else sel -= sizAir + 1;
    } sel -= 1;

    if (sel >= 0 && sel != m_profile["anime"][(categ) ? "Airing" : "Finished"].size()-1) {
      std::string aux = m_profile["anime"][(categ) ? "Airing" : "Finished"][sel+1].dump(2);
      m_profile["anime"][(categ) ? "Airing" : "Finished"][sel+1] = m_profile["anime"][(categ) ? "Airing" : "Finished"][sel];
      m_profile["anime"][(categ) ? "Airing" : "Finished"][sel] = json::parse(aux);

      m_profile["anime"][(categ) ? "Airing" : "Finished"][sel]["id"] = sel;
      m_profile["anime"][(categ) ? "Airing" : "Finished"][sel+1]["id"] = sel+1;

      writeProfile();
      loadProfile();
  
      animeListBox->SetSelection(c_sel+1);
    }
  } catch (...) { std::fprintf(stderr, "Unable to move anime DOWN!\n"); }
}

void MainWindow::OnCheck(wxCommandEvent &event) {
  try {
    int siz = m_profile["anime"]["Airing"].size();
    if (siz) { int sel;
      for (sel = 0; sel < siz; ++sel) { LoadPreview(sel, 1, 0); previewPanel->refreshAnime(0, 0); }
      animeListBox->SetSelection(1); LoadPreview(0, 1, 1);
    }
  } catch (...) { std::fprintf(stderr, "Unable to refresh anime!\n"); }
}

void MainWindow::ImportDir(wxCommandEvent &event) {
  wxDirDialog *folderDialog = new wxDirDialog(this, "Choose input directory", m_profile["settings"]["defaultPath"].get<std::string>(), wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
  if (folderDialog->ShowModal() == wxID_OK) {
    std::string path = folderDialog->GetPath().ToStdString(), pathA;
    std::string name, name_c, link_mal, link_gogo; int mal;
    SearchDialog *searchDialog;

    for (const auto &entry : std::filesystem::directory_iterator(path)) {
      name = truncateDirectoryName(entry.path().filename().string()); name_c = name;

      if (animeListBox->FindString(name) == -1) { mal = -1;
        searchDialog = new SearchDialog(wxT("Search Dialog"), 400, 300, &name_c, &mal, &link_gogo, 0);
        searchDialog->Show();

        if (mal != -1) { link_mal = std::to_string(mal); pathA = entry.path().string();
          // Get details from MAL
          bool status; std::string image_link;
          int total_ep = getAnimeDetails(link_mal, status, image_link);

          char category[12];
          if (status) std::strcpy(category, "Airing"); else std::strcpy(category, "Finished");

          int i = m_profile["anime"][category].size();
          m_profile["anime"][category][i]["name"] = name;
          m_profile["anime"][category][i]["path"] = pathA;
          m_profile["anime"][category][i]["id"] = i;

          m_active["name"] = name;
          m_active["link_mal"] = link_mal;
          m_active["link_gogo"] = link_gogo;
          m_active["diskEp"] = getLastDownEp(pathA);
          m_active["airedEp"] = getLastGogoEp(link_gogo);
          m_active["totalEp"] = total_ep;
          m_active["link_img"] = image_link;
          writeJson(std::string(pathA + "/data.json").c_str(), m_active);
          urlImageToDisk(image_link, pathA);

          writeProfile();
          loadProfile();
        }
        
        delete searchDialog;
      }
    }
  }
  delete folderDialog;
}

void MainWindow::SetDefaultPath(wxCommandEvent& event) {
  wxDirDialog *folderDialog = new wxDirDialog(this, "Choose default directory", "", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
  if (folderDialog->ShowModal() == wxID_OK) {
    m_profile["settings"]["defaultPath"] = folderDialog->GetPath();
    writeProfile();
  }
}

void MainWindow::LoadPreview(int sel, bool categ, bool full) {
  try {
    std::string path = m_profile["anime"][(categ) ? "Airing" : "Finished"][sel]["path"].get<std::string>();
    previewPanel->animeName->ChangeValue(m_profile["anime"][(categ) ? "Airing" : "Finished"][sel]["name"].get<std::string>());
    previewPanel->animePath->ChangeValue(path);

    readJson(std::string(path + "/data.json").c_str(), m_active);
    previewPanel->animeLink_MAL->ChangeValue(m_active["link_mal"].get<std::string>());
    previewPanel->animeLink_Gogo->ChangeValue(m_active["link_gogo"].get<std::string>());
    if (full) {
      previewPanel->animeStatus->SetLabel((categ) ? "Airing" : "Finished");
      previewPanel->animeEp->SetLabel(std::string(std::to_string(m_active["diskEp"].get<int>()) + " : " + std::to_string(m_active["airedEp"].get<int>()) + " : " + std::to_string(m_active["totalEp"].get<int>())));

      path += "/image.jpg";
      std::FILE *fp; fp = std::fopen(path.c_str(), "r");
      if (fp) previewPanel->animeLogo->SetBitmap(wxImage(path, wxBITMAP_TYPE_JPEG));
      else previewPanel->animeLogo->SetBitmap(wxImage(225, 319));

      previewPanel->OnLoadEp();
    }
  } catch (...) { std::fprintf(stderr, "Unable to load anime preview!\n"); }
}
void MainWindow::LoadPreview(wxCommandEvent &event) {
  try {
    int sel = animeListBox->GetSelection(), c_sel = sel;

    int sizAir = m_profile["anime"]["Airing"].size(); bool categ = 0;
    if (sizAir) {
      if (sel <= sizAir) categ = 1;
      else sel -= sizAir + 1;
    } sel -= 1;

    if (sel >= 0) LoadPreview(sel, categ, 1);
  } catch (...) { std::fprintf(stderr, "Unable to load anime preview!\n"); }
}


PreviewPanel::PreviewPanel(wxPanel *parent) : wxPanel(parent, -1) { m_parent = parent;
  MainWindow *mw = (MainWindow *) parent->GetParent();

  wxPanel *animeDetails = new wxPanel(this, -1);
  wxPanel *imagePanel = new wxPanel(this, -1);
  wxPanel *epPanel = new wxPanel(this, -1);

  wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
  wxBoxSizer *vbox1 = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *hbox11 = new wxBoxSizer(wxHORIZONTAL);
  wxBoxSizer *hbox12 = new wxBoxSizer(wxHORIZONTAL);
  wxBoxSizer *hbox13 = new wxBoxSizer(wxHORIZONTAL);
  wxBoxSizer *hbox14 = new wxBoxSizer(wxHORIZONTAL);
  wxBoxSizer *hbox15 = new wxBoxSizer(wxHORIZONTAL);
  wxBoxSizer *hbox16 = new wxBoxSizer(wxHORIZONTAL);
  wxBoxSizer *hbox17 = new wxBoxSizer(wxHORIZONTAL);
  wxBoxSizer *vbox2 = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *vbox3 = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *hbox31 = new wxBoxSizer(wxHORIZONTAL);
  wxBoxSizer *hbox32 = new wxBoxSizer(wxHORIZONTAL);

  wxStaticText *template_AnimeDetails = new wxStaticText(animeDetails, -1, wxT("Anime Details:")); template_AnimeDetails->SetFont(wxFont(16, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_NORMAL, 1));
  refreshButton = new wxButton(animeDetails, -1, wxT("Refresh Data"));
  editButton = new wxToggleButton(animeDetails, -1, wxT("Edit Mode"));

  hbox11->Add(template_AnimeDetails, 1, wxCENTRE);
  hbox11->Add(refreshButton, 0.5, wxCENTRE | wxLEFT, 40);
  hbox11->Add(editButton, 0.5, wxCENTRE | wxLEFT, 10);
  vbox1->Add(hbox11, 1, wxBOTTOM, 10);

  const int fieldWidth = mw->m_width / 7 * 5 / 3 / 2;

  wxStaticText *template_AnimeName = new wxStaticText(animeDetails, -1, wxT("Name: "), wxPoint(-1, -1), wxSize(fieldWidth, -1)); template_AnimeName->SetFont(wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
  animeName = new wxTextCtrl(animeDetails, -1, wxT(""), wxPoint(-1, -1), wxSize(-1, -1), wxTE_READONLY);
  hbox12->Add(template_AnimeName, 1, wxALIGN_CENTRE | wxALL, 10); hbox12->Add(animeName, 2, wxALIGN_CENTRE); vbox1->Add(hbox12, 1, wxALIGN_TOP);

  wxStaticText *template_AnimePath = new wxStaticText(animeDetails, -1, wxT("Path: "), wxPoint(-1, -1), wxSize(fieldWidth, -1)); template_AnimePath->SetFont(wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
  animePath = new wxTextCtrl(animeDetails, -1, wxT(""), wxPoint(-1, -1), wxSize(-1, -1), wxTE_READONLY);
  hbox13->Add(template_AnimePath, 1, wxALIGN_CENTRE | wxALL, 10); hbox13->Add(animePath, 2, wxALIGN_CENTRE); vbox1->Add(hbox13, 1, wxALIGN_TOP);

  wxStaticText *template_AnimeLink_MAL = new wxStaticText(animeDetails, -1, wxT("MAL Link: "), wxPoint(-1, -1), wxSize(fieldWidth, -1)); template_AnimeLink_MAL->SetFont(wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
  animeLink_MAL = new wxTextCtrl(animeDetails, -1, wxT(""), wxPoint(-1, -1), wxSize(-1, -1), wxTE_READONLY);
  hbox14->Add(template_AnimeLink_MAL, 1, wxALIGN_CENTRE | wxALL, 10); hbox14->Add(animeLink_MAL, 2, wxALIGN_CENTRE); vbox1->Add(hbox14, 1, wxALIGN_TOP);

  wxStaticText *template_AnimeLink_Gogo = new wxStaticText(animeDetails, -1, wxT("Gogo Link: "), wxPoint(-1, -1), wxSize(fieldWidth, -1)); template_AnimeLink_Gogo->SetFont(wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
  animeLink_Gogo = new wxTextCtrl(animeDetails, -1, wxT(""), wxPoint(-1, -1), wxSize(-1, -1), wxTE_READONLY);
  hbox15->Add(template_AnimeLink_Gogo, 1, wxALIGN_CENTRE | wxALL, 10); hbox15->Add(animeLink_Gogo, 2, wxALIGN_CENTRE); vbox1->Add(hbox15, 1, wxALIGN_TOP);

  wxStaticText *template_AnimeStatus = new wxStaticText(animeDetails, -1, wxT("Status: "), wxPoint(-1, -1), wxSize(fieldWidth, -1)); template_AnimeStatus->SetFont(wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
  animeStatus = new wxStaticText(animeDetails, -1, wxT("")); animeStatus->SetFont(wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_NORMAL));
  hbox16->Add(template_AnimeStatus, 1, wxALIGN_CENTRE | wxALL, 10); hbox16->Add(animeStatus, 2, wxALIGN_CENTRE | wxLEFT, 5); vbox1->Add(hbox16, 1, wxALIGN_TOP);

  wxStaticText *template_AnimeEp = new wxStaticText(animeDetails, -1, wxT("Episodes: "), wxPoint(-1, -1), wxSize(fieldWidth, -1)); template_AnimeEp->SetFont(wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
  animeEp = new wxStaticText(animeDetails, -1, wxT("")); animeEp->SetFont(wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_NORMAL));
  hbox17->Add(template_AnimeEp, 1, wxALIGN_CENTRE | wxALL, 10); hbox17->Add(animeEp, 1, wxALIGN_CENTRE | wxLEFT, 5);
  checkButton = new wxButton(animeDetails, -1, wxT("Check Ep")); hbox17->Add(checkButton, 0.5, wxALIGN_CENTRE | wxLEFT, fieldWidth/3+10);
  vbox1->Add(hbox17, 1, wxALIGN_TOP);

  animeDetails->SetSizer(vbox1);


  wxJPEGHandler *jpegHandler = new wxJPEGHandler;
  wxImage::AddHandler(jpegHandler);
  animeLogo = new wxStaticBitmap(imagePanel, -1, wxBitmap(wxImage(225, 319)));
  vbox2->Add(animeLogo, 1);
  imagePanel->SetSizer(vbox2);

  hbox->Add(animeDetails, 1); hbox->Add(imagePanel, 1, wxLEFT | wxBOTTOM, 20);
  vbox->Add(hbox, 2);


  epPanell = new EpisodePanel(epPanel);
  vbox3->Add(epPanell, 1, wxEXPAND);
  epPanel->SetSizer(vbox3);

  vbox->Add(epPanel, 1, wxCENTRE | wxEXPAND);

  SetSizer(vbox);

  refreshButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &PreviewPanel::OnRefresh, this);
  checkButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &PreviewPanel::OnCheck, this);
  editButton->Bind(wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, &PreviewPanel::OnEdit, this);
}

void PreviewPanel::OnRefresh(wxCommandEvent& event) { refreshAnime(1, 1); }
void PreviewPanel::OnCheck(wxCommandEvent& event) { refreshAnime(0, 1); }
void PreviewPanel::refreshAnime(bool refresh, bool display) {
  try { MainWindow *mw = (MainWindow *) m_parent->GetParent();
    int sel = mw->animeListBox->FindString(animeName->GetValue()), c_sel = sel;
    int siz = mw->m_profile["anime"]["Airing"].size(); bool categ = 0;
    if (siz) {
      if (sel <= siz) categ = 1;
      else sel -= siz + 1;
    } sel -= 1;

    if (sel >= 0) {
      std::string path = animePath->GetValue().ToStdString();
      std::string link_mal = animeLink_MAL->GetValue().ToStdString();
      std::string link_gogo = animeLink_Gogo->GetValue().ToStdString();

      bool status;

      mw->m_active["diskEp"] = getLastDownEp(path);
      mw->m_active["airedEp"] = getLastGogoEp(link_gogo);

      if (refresh) {
        std::string image_link;
        int total_ep = getAnimeDetails(link_mal, status, image_link);

        mw->m_active["totalEp"] = total_ep;

        mw->m_active["link_img"] = image_link;
        writeJson(std::string(path + "/data.json").c_str(), mw->m_active);
        urlImageToDisk(image_link, path);
      } else status = getAnimeStatus(link_mal);

      writeJson(std::string(path + "/data.json").c_str(), mw->m_active);
    
      bool ok = 0;
      if (status != categ) { ok = 1; switchCategory(sel, categ); }

      if (ok) sel = mw->m_profile["anime"][(status) ? "Airing" : "Finished"].size() - 1;
      if (display) mw->LoadPreview(sel, status, 1);
      
      mw->writeProfile();
      mw->loadProfile();

      if (status) mw->animeListBox->SetSelection(1 + sel);
      else mw->animeListBox->SetSelection(mw->m_profile["anime"]["Airing"].size() + 2 + sel);
    }

  } catch (...) { std::fprintf(stderr, "Unable to refresh anime!\n"); }
}
void PreviewPanel::switchCategory(int sel, bool categ) {
  try { MainWindow *mw = (MainWindow *) m_parent->GetParent();
    // Copy entry
    std::string aux = mw->m_profile["anime"][(categ) ? "Airing" : "Finished"][sel].dump(2);

    // Add Entry
    mw->m_profile["anime"][(!categ) ? "Airing" : "Finished"].push_back(mw->m_profile["anime"][(categ) ? "Airing" : "Finished"][sel]);
    mw->m_profile["anime"][(!categ) ? "Airing" : "Finished"].back()["id"] = mw->m_profile["anime"][(!categ) ? "Airing" : "Finished"].size()-1;

    // Delete entry
    mw->m_profile["anime"][(categ) ? "Airing" : "Finished"].erase(sel);
    int siz = mw->m_profile["anime"][(categ) ? "Airing" : "Finished"].size();
    int i;
    for (i = sel; i < siz; ++i) mw->m_profile["anime"][(categ) ? "Airing" : "Finished"][i]["id"] = i;
  } catch (...) { std::fprintf(stderr, "Unable to switch category!\n"); }
}
void PreviewPanel::OnEdit(wxCommandEvent& event) {
  try {
    if (editButton->GetValue()) { animeNameS = animeName->GetValue();
      animeName->SetEditable(1); animePath->SetEditable(1); animeLink_MAL->SetEditable(1); animeLink_Gogo->SetEditable(1);
    } else { MainWindow *mw = (MainWindow *) m_parent->GetParent();
      animeName->SetEditable(0); animePath->SetEditable(0); animeLink_MAL->SetEditable(0); animeLink_Gogo->SetEditable(0);

      int sel = mw->animeListBox->FindString(animeNameS), c_sel = sel;

      int siz = mw->m_profile["anime"]["Airing"].size(); bool categ = 0;
      if (siz) {
        if (sel <= siz) categ = 1;
        else sel -= siz + 1;
      } sel -= 1;

      if (sel >= 0) { std::string path = animePath->GetValue().ToStdString();
        mw->m_profile["anime"][(categ) ? "Airing" : "Finished"][sel]["name"] = animeName->GetValue().ToStdString();
        mw->m_profile["anime"][(categ) ? "Airing" : "Finished"][sel]["path"] = animePath->GetValue().ToStdString();

        readJson(std::string(animePath->GetValue().ToStdString() + "/data.json").c_str(), mw->m_active);
        mw->m_active["name"] = animeName->GetValue().ToStdString();
        mw->m_active["link_mal"] = animeLink_MAL->GetValue().ToStdString();
        mw->m_active["link_gogo"] = animeLink_Gogo->GetValue().ToStdString();
        writeJson(std::string(animePath->GetValue().ToStdString() + "/data.json").c_str(), mw->m_active);

        mw->writeProfile();
        mw->loadProfile();

        mw->animeListBox->SetSelection(c_sel);
      }
    }
  } catch (...) { std::fprintf(stderr, "Unable to edit anime!\n"); }
}

void PreviewPanel::OnLoadEp() {
  try { MainWindow *mw = (MainWindow *) m_parent->GetParent();
  epPanell->epList->ClearAll();

  int diskEp = mw->m_active["diskEp"].get<int>();
  int airedEp = mw->m_active["airedEp"].get<int>();

  wxListItem item;

  int i = 1;
  for (; i <= diskEp && i <= airedEp/2; ++i) epPanell->epList->InsertItem(i-1, std::string("Local EP: " + std::to_string(i)));
  for (; i <= airedEp/2; ++i) epPanell->epList->InsertItem(i-1, std::string("Down EP: " + std::to_string(i)));

  for (; i <= diskEp && i <= airedEp; ++i) epPanell->epList->InsertItem(i-1, std::string("Local EP: " + std::to_string(i)));
  for (; i <= airedEp; ++i) epPanell->epList->InsertItem(i-1, std::string("Down EP: " + std::to_string(i)));

  } catch (...) { std::fprintf(stderr, "Unable to load anime episodes!\n"); }
}

EpisodePanel::EpisodePanel(wxPanel *parent) : wxPanel(parent, -1) { m_parent = parent;
  wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);

  epList = new wxListCtrl(this, -1, wxPoint(-1, -1), wxSize(-1, -1), wxLC_LIST | wxLC_SINGLE_SEL);
  vbox->Add(epList, 1, wxCENTRE | wxEXPAND);

  epList->Bind(wxEVT_LIST_ITEM_ACTIVATED, &EpisodePanel::OnOpen, this);

  SetSizer(vbox);
}

void EpisodePanel::OnOpen() {
  try {
    long int sel = epList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (sel >= 0) { PreviewPanel *pp = (PreviewPanel *) m_parent->GetParent(); MainWindow *mw = (MainWindow *) pp->m_parent->GetParent();
      std::string path(pp->animePath->GetValue().ToStdString() + "/EP." + std::to_string(sel+1) + ".mp4");

      std::filesystem::path f(path);
      if (std::filesystem::exists(f)) std::system(std::string("mpv --fs \"" + path + '\"').c_str());
      else { std::string link;
        try {
          link = mw->m_active["link_gogo_ep"].get<std::string>();
        } catch (...) { link = pp->animeLink_Gogo->GetValue().ToStdString(); }

        long int res;
        link = getGogoEpDownLink(link, sel+1, res);
        if (res) {
          link = wxGetTextFromUser(wxT("Gogo EP link")).ToStdString();
          if (link.empty()) return;

          if (link.back() == '/') link.pop_back();
          size_t epPos1 = link.find_last_of('/');
          if (epPos1 != std::string::npos) {
            size_t epPos2 = link.find("-episode-", epPos1);
            if (epPos2 != std::string::npos) { epPos2 -= 1;
              link = link.substr(epPos1+1, epPos2-epPos1);

              mw->m_active["link_gogo_ep"] = link;
              writeJson(std::string(pp->animePath->GetValue().ToStdString() + "/data.json").c_str(), mw->m_active);

              OnOpen(); return;
            } else { std::fprintf(stderr, "Error when trying to find the ep string pos 2!\n"); return; }
          } else { std::fprintf(stderr, "Error when trying to find the ep string pos 1!\n"); return; }
        }

        if (wxTheClipboard->Open()) {
          wxTheClipboard->SetData(new wxTextDataObject(link));
          wxTheClipboard->Close();
        }
      }
    }
  } catch (...) { std::fprintf(stderr, "Unable to open anime episode!\n"); }
}
void EpisodePanel::OnOpen(wxCommandEvent& event) { OnOpen(); }
