// #include <cstdint>
// #include <ctime>

#include <paf.h>
// #include <paf/std/stdc>
// #include <paf/std/string.h>
// #include <paf/widget/core/event.h>
#include <psp2/io/fcntl.h>
// #include <psp2/io/stat.h>
#include <psp2/kernel/clib.h>
#include <psp2/kernel/modulemgr.h>
// #include <psp2/kernel/processmgr.h>
#include <psp2/kernel/threadmgr/thread.h>
#include <psp2/sysmodule.h>

// #include <curlpp/cURLpp.hpp>
// #include <psp2common/kernel/iofilemgr.h>
// #include <pthread.h>
#include <rapidxml.hpp>
#include <rapidxml_print.hpp>

using namespace rapidxml;
void rapidxml::parse_error_handler(const char *what, void *where) {
  sceClibPrintf("Failed to parse XML: %s at location %p\n", what, where);
  sceKernelExitThread(1);
}

static const char *DEFAULT_FEEDS_PATH = "app0:/default_feeds.opml";
static const char *USER_FEEDS_PATH = "savedata0:/feeds.opml";
static const char *UNREAD_ARTICLES_PATH = "savedata0:/unread.xml";
static const char *READ_ARTICLES_PATH = "savedata0:/read.xml";

static const int LIBHTTP_POOLSIZE = (20 * 1000);

#define MAX(X, Y) X > Y ? X : Y

// this converts to string
#define STR_(X) #X
// this makes sure the argument is expanded before converting to string
#define STR(X) STR_(X)

static const char *USER_AGENT =
#ifdef VITA_VERSION
    "vita_rss/" STR(VITA_VERSION) " (+https://github.com/cafehaine/vita_rss)";
#else
    "vita_rss/XX.XX (+https://github.com/cafehaine/vita_rss)";
#endif

paf::Framework *g_fw;
paf::Plugin *g_samplePlugin;
paf::ui::Scene *g_rootPage;

paf::ui::Plane *g_pPlane;
paf::ui::Plane *g_pPlaneMain;
paf::ui::Scene *g_pScene;
paf::ui::CornerButton *g_pReturnCornerButton;
paf::ui::CornerButton *g_pSettingsCornerButton;
paf::ui::Plane *g_pPlaneRoot;

class State {
public:
  virtual void enter();
  virtual ~State() = default;
};

class MenuItemFactory : public paf::ui::listview::ItemFactory {
private:
  uint16_t index;

public:
  std::vector<std::pair<paf::wstring, std::optional<paf::ui::HandlerCB>>>
      entries;
  MenuItemFactory(
      std::vector<std::pair<paf::wstring, std::optional<paf::ui::HandlerCB>>>
          map)
      : entries{map} {
    index = 0;
  };

  void Start(StartParam &param) {
    param.list_item->Show(paf::common::transition::Type_FadeinSlow);
  }

  void Stop(StopParam &param) {
    param.list_item->Hide(paf::common::transition::Type_FadeinSlow);
  }

  paf::ui::ListItem *Create(CreateParam &param) {

    sceClibPrintf("Creating list item %d\n", index);

    paf::Plugin::TemplateOpenParam openParam;
    int res = g_samplePlugin->TemplateOpen(
        param.parent, "_sample_template_list_item", openParam);
    if (res != 0) {
      sceClibPrintf("TemplateOpen 0x%X\n", res);
    }

    paf::ui::ListItem *list_item = (paf::ui::ListItem *)param.parent->GetChild(
        param.parent->GetChildrenNum() - 1);
    paf::ui::Widget *button = list_item->FindChild("button");

    auto entry = entries[index];
    index += 1;
    button->SetString(entry.first.c_str());
    if (!entry.second.has_value()) {
      button->SetShowAlpha(0.3);
    } else {
      button->SetEventCallback(paf::ui::ButtonBase::CB_BTN_DECIDE,
                               entry.second.value());
    }
    return list_item;
  }
};

class MainMenu : public State {
public:
  virtual void enter() {
    paf::common::transition::Do(
        0.0f, g_pPlaneRoot, paf::common::transition::Type::Type_3D_SlideToBack1,
        false, false);

    paf::ui::Plane *pPlaneSub =
        (paf::ui::Plane *)g_pPlaneMain->FindChild("plane_sample_black_sub");
    pPlaneSub->Show(NULL, 2);
    paf::common::transition::Do(
        0.0f, pPlaneSub, paf::common::transition::Type::Type_3D_SlideFromFront,
        false, false);

    g_pReturnCornerButton->Show(NULL, 2);
    paf::common::transition::Do(0.0f, g_pReturnCornerButton,
                                paf::common::transition::Type::Type_Popup5,
                                false, false);
    g_pSettingsCornerButton->Show(NULL, 2);
    paf::common::transition::Do(0.0f, g_pSettingsCornerButton,
                                paf::common::transition::Type::Type_Popup5,
                                false, false);
  }
  virtual ~MainMenu();
};

class ArticleList : public State {
public:
  virtual void enter();
  virtual ~ArticleList();
};

class ArticleView : public State {
public:
  virtual void enter();
  virtual ~ArticleView();
};

class ArticleWebview : public ArticleView {
public:
  virtual void enter();
  virtual ~ArticleWebview();
};

class SettingsView : public State {
public:
  virtual void enter();
  virtual ~SettingsView();
};

State *g_currentState;

/*
{

  {
    paf::common::transition::DoReverse(
        0.0f, g_pReturnCornerButton,
        paf::common::transition::Type::Type_Popup5, false, false);
    paf::common::transition::DoReverse(
        0.0f, g_pSettingsCornerButton,
        paf::common::transition::Type::Type_Popup5, false, false);
    paf::Timer *pTimer =
        new paf::Timer(500.0f, paf::Timer::Func::FUNC_BOUNCE_IN);
    g_pReturnCornerButton->Hide(pTimer, 2);
    paf::Timer *pTimer2 =
        new paf::Timer(500.0f, paf::Timer::Func::FUNC_BOUNCE_IN);
    g_pSettingsCornerButton->Hide(pTimer2, 2);
  }

  {
    paf::ui::Plane *pPlaneSub =
        (paf::ui::Plane *)g_pPlaneMain->FindChild("plane_sample_black_sub");
    paf::Timer *pTimer =
        new paf::Timer(500.0f, paf::Timer::Func::FUNC_BOUNCE_IN);
    pPlaneSub->Hide(pTimer, 2);
    paf::common::transition::DoReverse(
        0.0f, pPlaneSub, paf::common::transition::Type::Type_FadeinFast,
        false, false);
  }

  paf::common::transition::DoReverse(
      0.0f, g_pPlaneRoot, paf::common::transition::Type::Type_3D_SlideToBack1,
      false, false);
}
*/

void paf_on_load(paf::Plugin *plugin) {
  sceClibPrintf("Paf on load…\n");

  g_samplePlugin = plugin;

  paf::Plugin::PageOpenParam pageOpenParam;
  paf::ui::Scene *pScene = plugin->PageOpen("page_main_bg", pageOpenParam);

  g_pReturnCornerButton = (paf::ui::CornerButton *)pScene->FindChild(
      "_sample_widget_corner_button_bottom_left");
  g_pReturnCornerButton->SetEventCallback(paf::ui::ButtonBase::CB_BTN_DECIDE,
                                          NULL);
  g_pReturnCornerButton->Hide(NULL, 2);
  g_pSettingsCornerButton = (paf::ui::CornerButton *)pScene->FindChild(
      "_sample_widget_corner_button_bottom_right");
  g_pSettingsCornerButton->SetEventCallback(paf::ui::ButtonBase::CB_BTN_DECIDE,
                                            NULL);
  g_pSettingsCornerButton->Hide(NULL, 2);

  g_pPlaneMain =
      (paf::ui::Plane *)pScene->FindChild("_sample_widget_plane_main");

  g_pPlaneRoot =
      (paf::ui::Plane *)g_pPlaneMain->FindChild("_sample_widget_plane_root");

  paf::ui::Plane *pPlaneSub =
      (paf::ui::Plane *)g_pPlaneMain->FindChild("plane_sample_black_sub");
  pPlaneSub->Hide(NULL, 2);

  paf::ui::ListView *list_view =
      (paf::ui::ListView *)g_pPlaneRoot->FindChild("list_view");

  sceClibPrintf("Initializing menu item factory…\n");
  auto itemFactory = new MenuItemFactory({{L"Unread articles", {}},
                                          {L"Favorited articles", {}},
                                          {L"Read articles", {}},
                                          {L"Settings", {}}});

  sceClibPrintf("Setting item factory for list view…\n");
  list_view->SetItemFactory(itemFactory);
  sceClibPrintf("Inserting segment (?)…\n");
  list_view->InsertSegment(0, 1);
  sceClibPrintf("Setting default cell size…\n");
  list_view->SetCellSizeDefault(0, {840.0f, 70.0f, 0.0f, 0.0f});

  // maybe add SetSegmentHeader here?
  sceClibPrintf("Setting segment layout type…\n");
  list_view->SetSegmentLayoutType(0, paf::ui::ListView::LAYOUT_TYPE_LIST);
  sceClibPrintf("Inserting cells…\n");
  list_view->InsertCell(0, 0, itemFactory->entries.size());
}

int paf_main(void) {
  sceClibPrintf("Paf main…\n");

  paf::Framework::InitParam fwParam;
  fwParam.mode = paf::Framework::Mode_Normal;

  paf::Framework *paf_fw = new paf::Framework(fwParam);
  if (paf_fw != NULL) {
    g_fw = paf_fw;

    paf_fw->LoadCommonResourceSync();

    paf::Plugin::InitParam pluginParam;

    pluginParam.name = "main";
    pluginParam.caller_name = "__main__";
    pluginParam.resource_file = "app0:/main.rco";
    pluginParam.init_func = NULL;
    pluginParam.start_func = paf_on_load;
    pluginParam.stop_func = NULL;
    pluginParam.exit_func = NULL;

    sceClibPrintf("Loading plugin…\n");
    paf::Plugin::LoadSync(pluginParam);
    sceClibPrintf("Paf framework run…\n");
    paf_fw->Run();
  }

  exit(0);

  return 0;
}
char sceUserMainThreadName[] = "vita_rss";
int sceUserMainThreadPriority = 0x10000100;
int sceUserMainThreadCpuAffinityMask = 0x70000;
SceSize sceUserMainThreadStackSize = 0x4000;

void operator delete(void *ptr, unsigned int n) { return sce_paf_free(ptr); }

int paf_main(void);

extern "C" {

typedef struct _ScePafInit { // size is 0x18
  SceSize global_heap_size;
  int a2;
  int a3;
  int cdlg_mode;
  int heap_opt_param1;
  int heap_opt_param2;
} ScePafInit;

bool file_exists(const char *path) {
  SceIoStat stat_result;
  return sceIoGetstat(path, &stat_result) >= 0;
}

void copy_file(const char *source_path, const char *dest_path) {
  sceClibPrintf("Copying %s over to %s…\n", source_path, dest_path);
  SceUID source_file = sceIoOpen(source_path, SCE_O_RDONLY, 0777);
  SceUID dest_file = sceIoOpen(dest_path, SCE_O_WRLOCK | SCE_O_CREAT, 0777);
  uint8_t buffer[1024];
  while (true) {
    SceSSize read = sceIoRead(source_file, buffer, 1024);
    sceIoWrite(dest_file, buffer, read);
    if (read < 1024) {
      break;
    }
  }
  sceIoClose(source_file);
  sceIoClose(dest_file);
  sceClibPrintf("Copy done!\n");
}

xml_document<char> *load_xml(const char *path) {
  xml_document<char> *document = new xml_document<char>;

  SceUID file = sceIoOpen(path, SceIoMode::SCE_O_RDONLY, 0777);
  auto file_size = sceIoLseek(file, 0, SceIoSeekMode::SCE_SEEK_END);
  sceIoLseek(file, 0, SceIoSeekMode::SCE_SEEK_SET);
  char *document_data = new char[file_size + 1];
  sceIoRead(file, document_data, file_size);
  sceIoClose(file);
  document->parse<0>(document_data);
  return document;
}

// void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp) {

  int load_res;
  ScePafInit init_param;
  SceSysmoduleOpt sysmodule_opt;

  init_param.global_heap_size = 0x1000000;
  init_param.a2 = 0xEA60;
  init_param.a3 = 0x40000;
  init_param.cdlg_mode = 0;
  init_param.heap_opt_param1 = 0;
  init_param.heap_opt_param2 = 0;

  load_res = 0xDEADBEEF;
  sysmodule_opt.flags = 0;
  sysmodule_opt.result = &load_res;

  int res = sceSysmoduleLoadModuleInternalWithArg(SCE_SYSMODULE_INTERNAL_PAF,
                                                  sizeof(init_param),
                                                  &init_param, &sysmodule_opt);
  if ((res | load_res) != 0) {
    sceClibPrintf("[PAF PRX Loader] Failed to load the PAF prx. (return value "
                  "0x%x, result code 0x%x )\n",
                  res, load_res);
    return 1;
  }

  // Load user feeds
  sceClibPrintf("Loading user feeds…\n");
  if (!file_exists(USER_FEEDS_PATH)) {
    // User feeds file does not exists, copy over default feeds
    copy_file(DEFAULT_FEEDS_PATH, USER_FEEDS_PATH);
  }
  xml_document<char> *user_feeds = load_xml(USER_FEEDS_PATH);

  // Load read articles
  sceClibPrintf("Loading read articles…\n");
  xml_document<char> *read_articles;
  if (!file_exists(READ_ARTICLES_PATH)) {
    read_articles = new xml_document<char>;
    xml_node<> *root_node =
        read_articles->allocate_node(rapidxml::node_element, "rss");
    read_articles->append_node(root_node);
  } else {
    read_articles = load_xml(READ_ARTICLES_PATH);
  }

  // Load unread articles
  xml_document<char> *unread_articles;
  sceClibPrintf("Loading unread articles…\n");
  if (!file_exists(UNREAD_ARTICLES_PATH)) {
    unread_articles = new xml_document<char>;
    xml_node<> *root_node =
        unread_articles->allocate_node(rapidxml::node_element, "rss");
    unread_articles->append_node(root_node);
  } else {
    unread_articles = load_xml(UNREAD_ARTICLES_PATH);
  }

  /*Initialize libhttp*/
  sceClibPrintf("Initialize LIBHTTP\n");
  sceHttpInit(LIBHTTP_POOLSIZE);
  /*Create template*/
  int template_id =
      sceHttpCreateTemplate(USER_AGENT, SCE_HTTP_VERSION_1_1, SCE_TRUE);
  /*Create connection*/
  int connection_id = sceHttpCreateConnectionWithURL(
      template_id, "https://pouet.chapril.org/@cafehaine.rss", SCE_TRUE);
  /*Create request*/
  int request_id = sceHttpCreateRequestWithURL(
      connection_id, SCE_HTTP_METHOD_GET,
      "https://pouet.chapril.org/@cafehaine.rss", 0);
  /*Send request and receive response header */
  int ret = sceHttpSendRequest(request_id, NULL, 0);
  int status_code;
  ret = sceHttpGetStatusCode(request_id, &status_code);
  unsigned long long content_length;
  ret = sceHttpGetResponseContentLength(request_id, &content_length);
  /*Receive message body*/
  char *buffer = new char[MAX(1024, content_length)];
  while (content_length > 0) {
    ret = sceHttpReadData(request_id, buffer, sizeof(buffer));
    if (ret < 0) {
      return 1;
    } else if (ret == 0) {
      break; /*The connection was closed*/
    } else {
      for (int counter = 0; counter < ret; counter++) {
        sceClibPrintf("%c", buffer[counter]);
      }
    }
  }
  sceClibPrintf("\n");
  /*Delete request*/
  sceHttpDeleteRequest(request_id);
  /*Library termination processing*/
  sceHttpDeleteConnection(connection_id);
  sceHttpDeleteTemplate(template_id);
  sceHttpTerm();

  paf_main();

  return SCE_KERNEL_START_SUCCESS;
}
}
