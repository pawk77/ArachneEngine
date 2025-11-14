#include <iostream>
#include <chrono>
#include <ctime>
#include <vector>
#include <windows.h>

#include "../../engine/thirdparty/glm/glm/glm.hpp"
#include "../../engine/thirdparty/glm/glm/gtc/matrix_transform.hpp"
#include "../../engine/src/application.hpp"
#include "../../engine/src/camera_manager.hpp"
#include "../../engine/src/texture_manager.hpp"
#include "../../engine/src/filesystem_manager.hpp"
#include "../../engine/src/render_manager.hpp"
#include "../../engine/src/physics_manager.hpp"
#include "../../engine/src/render_context.hpp"
#include "../../engine/src/sound_context.hpp"
#include "../../engine/src/sound_manager.hpp"
#include "../../engine/src/font_manager.hpp"
#include "../../engine/src/types.hpp"
#include "winapi/window.hpp"
#include "winapi/listview.hpp"
#include "winapi/groupbox.hpp"
#include "winapi/textbox.hpp"
#include "winapi/label.hpp"
#include "winapi/button.hpp"
#include "ray_hit.hpp"
#include "editor_types.hpp"

using namespace realware::core;
using namespace realware::app;
using namespace realware::render;
using namespace realware::font;
using namespace realware::sound;
using namespace realware::physics;
using namespace realware::editor;
using namespace realware::game;

mRender* renderManager;
mTexture* textureManager;
mSound* soundManager;
mCamera* cameraManager;
//mFileSystem* fileSystemManager;
mFont* fontManager;
mPhysics* physicsManager;
cApplication* editorApp = nullptr;

//realware::core::u64 editorUniqueID = 0;
std::vector<sVertexBufferGeometry*> editorGeometriesToDraw;
eSelectMode editorSelectMode = eSelectMode::CREATE;
//entity editorSelectedEntity = 0;
//entity editorCopyEntity = 0;
int editorSelectedAssetIndex = -1;
int editorUsedAssetIndex = -1;
glm::vec3 editorPosition = glm::vec3(0.0f), editorRotation = glm::vec3(0.0f), editorScale = glm::vec3(0.0f);
sTextboxLabel editorPositionX; sTextboxLabel editorPositionY; sTextboxLabel editorPositionZ;
sTextboxLabel editorRotationX; sTextboxLabel editorRotationY; sTextboxLabel editorRotationZ;
sTextboxLabel editorScaleX; sTextboxLabel editorScaleY; sTextboxLabel editorScaleZ;
sTextboxLabel editorLightScale;
sTextboxLabel editorLightColor;
sTextboxLabel editorLightAttenuation;
sTextboxLabel editorScript;
sTextboxLabel editorWindowEntityName;
sTextboxLabel editorWindowEntityTexture;
sTextboxLabel editorWindowEntityGeometry;
sTextboxLabel editorWindowEntityDiffuseColor;
sTextboxLabel editorWindowSoundName;
sTextboxLabel editorWindowSoundFile;
sTextboxLabel editorWindowScriptName;
sTextboxLabel editorWindowScriptCode;
sTextboxLabel editorWindowAssetSearch;
cEditorWindow* editorWindowMain = nullptr;
cEditorWindow* editorWindowAsset = nullptr;
cEditorWindow* editorWindowEntity = nullptr;
cEditorWindow* editorWindowSound = nullptr;
cEditorWindow* editorWindowScript = nullptr;
cEditorListView* editorWindowAssetListView = nullptr;
cEditorButton* editorWindowAssetEntitiesButton = nullptr;
cEditorButton* editorWindowAssetSoundsButton = nullptr;
cEditorButton* editorWindowAssetScriptsButton = nullptr;
cEditorButton* editorWindowEntityOKButton = nullptr;
cEditorButton* editorWindowEntityCloseButton = nullptr;
cEditorButton* editorWindowSoundOKButton = nullptr;
cEditorButton* editorWindowSoundCloseButton = nullptr;
cEditorButton* editorWindowScriptOKButton = nullptr;
cEditorButton* editorWindowScriptCloseButton = nullptr;
eAssetSelectedType editorWindowAssetSelectedType = eAssetSelectedType::ENTITY;
std::vector<std::vector<sAsset>> editorWindowAssetData((int)eAssetSelectedType::_COUNT);

void EditorUpdateTextboxTransform();//(sCTransform* transform);
void EditorUpdateTextboxLight();//(sCLight* light);
void EditorUpdateEntityFields();//(cScene* scene);
void EditorWindowAssetShowPopupmenu();//(realware::core::boolean rmbPress);
void EditorWindowAssetDeleteItem(
    cApplication* app,
    //cScene* scene,
    const eAssetSelectedType& type,
    int assetIndex
);
void EditorAssetLoadData(eAssetSelectedType type, sAsset& asset);
void EditorWindowEntityUpdate(int assetIndex);
void EditorWindowEntitySave();//(cApplication* app, cScene* scene, int assetIndex);
void EditorWindowSoundUpdate(int assetIndex);
void EditorWindowSoundSave();//(cApplication* app, cScene* scene, int assetIndex);
void EditorWindowScriptUpdate(int assetIndex);
void EditorWindowScriptSave();//(cApplication* app, cScene* scene, int assetIndex);
void EditorWindowRenderEntityLogic(
    cApplication* app
    //cScene* scene,
    //entity camera,
    //realware::core::boolean lmbPress,
    //realware::core::boolean rmbPress
);
void EditorWindowObjectLogic();//;(cApplication* app, cScene* scene);
void EditorWindowAssetLogic();
void EditorNewMap();//(cApplication* app, cScene* scene);
void EditorOpenMap();//(cApplication* app, cScene* scene, const std::string& filename);
void EditorSaveMap();//(cApplication* app, cScene* scene, const std::string& filename);
void EditorNewPlugin();//(cApplication* app, cScene* scene);
void EditorOpenPlugin();//(cApplication* app, cScene* scene, const std::string& filename);
void EditorSavePlugin();//(cApplication* app, cScene* scene, const std::string& filename);
std::string EditorGetExeFolder();

class MyApp : public cApplication
{

public:
    MyApp(const sApplicationDescriptor& desc) : cApplication((sApplicationDescriptor*)&desc) {}
    ~MyApp() {}

    virtual void Start() override final
    {
        editorApp = this;

        // Initialize managers
        textureManager = new mTexture(this, _renderContext);
        soundManager = new mSound(this, _soundContext);
        renderManager = new mRender(this, _renderContext);
        cameraManager = new mCamera(this);
        physicsManager = new mPhysics(this);

        _camera->CreateCamera();
        _camera->SetMoveSpeed(5.0f);

        // Triangle primitive
        //auto plane = renderManager->CreateModel("data/models/plane.dae");
        //m_geomPlane = renderManager->CreateGeometry(
        //    plane->Format,
        //    plane->VerticesByteSize,
        //    plane->Vertices,
        //    plane->IndicesByteSize,
        //    plane->Indices
        //);

        auto dirtTexture = textureManager->CreateTexture("data/textures/dirt.png", "DirtTexture");
        //m_taburetTexture = textureManager->AddTexture("data/textures/taburet2.png", "TaburetTexture");

        // Editor windows
        // Main window
        float offset = this->GetWindowSize().x * 0.0025f;
        glm::vec2 mainWindowSize = glm::vec2(
                (this->GetWindowSize().x / 2.0f) - (this->GetWindowSize().x * 0.025f) - (this->GetWindowSize().x * 0.0125f),
                (this->GetWindowSize().y * 0.142f)
        );
        editorWindowMain = new cEditorWindow(
            nullptr,
            "MainWindow",
            "RealWare Editor",
            glm::vec2(0.0f),
            mainWindowSize
        );
        auto menus = editorWindowMain->AddMenu({ "File" });
        editorWindowMain->AddSubmenu(menus[0], 1, "New plugin");
        editorWindowMain->AddSubmenu(menus[0], 2, "Open plugin");
        editorWindowMain->AddSubmenu(menus[0], 3, "Save plugin");
        editorWindowMain->AddSubmenuSeparator(menus[0]);
        editorWindowMain->AddSubmenu(menus[0], 4, "New map");
        editorWindowMain->AddSubmenu(menus[0], 5, "Open map");
        editorWindowMain->AddSubmenu(menus[0], 6, "Save map");
        editorWindowMain->AddSubmenuSeparator(menus[0]);
        editorWindowMain->AddSubmenu(menus[0], 7, "Exit");

        RemoveWindowSysmenu(editorWindowMain->GetHWND());

        // Asset window
        glm::vec2 assetWindowSize = glm::vec2(
            (this->GetWindowSize().x / 2.0f) - (this->GetWindowSize().x * 0.025f) - (this->GetWindowSize().x * 0.0125f),
            this->GetWindowSize().y - (this->GetWindowSize().y * 0.2f)
        );
        editorWindowAsset = new cEditorWindow(
            nullptr,
            "AssetWindow",
            "Assets",
            glm::vec2(0.0f, (this->GetWindowSize().y * 0.1f) + (this->GetWindowSize().y * 0.05f)),
            assetWindowSize
        );
        RemoveWindowSysmenu(editorWindowAsset->GetHWND());
        editorWindowAssetListView = new cEditorListView(
            editorWindowAsset->GetHWND(),
            "AssetListView",
            "Asset view",
            glm::vec2(0.0f, offset * 20.0f),
            offset
        );
        editorWindowAssetListView->AddColumn(0, "Name", 150);

        editorWindowAssetEntitiesButton = new cEditorButton(
            editorWindowAsset->GetHWND(),
            "Entities",
            glm::vec2(offset * 2.0f),
            glm::vec2(offset * 25.0f, offset * 7.0f)
        );
        editorWindowAssetSoundsButton = new cEditorButton(
            editorWindowAsset->GetHWND(),
            "Sounds",
            glm::vec2(offset * 28.0f, offset * 2.0f),
            glm::vec2(offset * 25.0f, offset * 7.0f)
        );
        editorWindowAssetScriptsButton = new cEditorButton(
            editorWindowAsset->GetHWND(),
            "Scripts",
            glm::vec2(offset * 55.0f, offset * 2.0f),
            glm::vec2(offset * 25.0f, offset * 7.0f)
        );

        editorWindowAssetSearch.Label = new cEditorLabel(editorWindowAsset->GetHWND(), "Search",
            glm::vec2(offset * 2.0f, offset * 13.0f), glm::vec2(offset * 15.0f, offset * 6.0f)
        );
        editorWindowAssetSearch.Textbox = new cEditorTextbox(editorWindowAsset->GetHWND(), "",
            glm::vec2(offset * 19.0f, offset * 13.0f), glm::vec2(offset * 55.0f, offset * 6.0f), types::K_FALSE, types::K_FALSE
        );

        // Entity window
        editorWindowEntity = new cEditorWindow(
            nullptr,
            "EntityWindow",
            "Entity",
            glm::vec2(0.0f),
            glm::vec2(offset * 120.0f, offset * 80.0f)
        );
        editorWindowEntity->Show(types::K_FALSE);
        RemoveWindowSysmenu(editorWindowEntity->GetHWND());

        editorWindowEntityOKButton = new cEditorButton(
            editorWindowEntity->GetHWND(),
            "OK",
            glm::vec2(offset * 106.0f, offset * 60.0f),
            glm::vec2(offset * 7.0f, offset * 7.0f)
        );
        editorWindowEntityCloseButton = new cEditorButton(
            editorWindowEntity->GetHWND(),
            "Close",
            glm::vec2(offset * 90.0f, offset * 60.0f),
            glm::vec2(offset * 15.0f, offset * 7.0f)
        );
        editorWindowEntityName.Label = new cEditorLabel(editorWindowEntity->GetHWND(), "Name",
            glm::vec2(offset * 3.0f, offset * 5.0f), glm::vec2(offset * 20.0f, offset * 7.0f)
        );
        editorWindowEntityName.Textbox = new cEditorTextbox(editorWindowEntity->GetHWND(), "",
            glm::vec2(offset * 25.0f, offset * 5.0f), glm::vec2(offset * 85.0f, offset * 7.0f), types::K_FALSE, types::K_FALSE
        );
        editorWindowEntityTexture.Label = new cEditorLabel(editorWindowEntity->GetHWND(), "Texture",
            glm::vec2(offset * 3.0f, offset * 14.0f), glm::vec2(offset * 20.0f, offset * 7.0f)
        );
        editorWindowEntityTexture.Textbox = new cEditorTextbox(editorWindowEntity->GetHWND(), "",
            glm::vec2(offset * 25.0f, offset * 14.0f), glm::vec2(offset * 85.0f, offset * 7.0f), types::K_FALSE, types::K_FALSE
        );
        editorWindowEntityGeometry.Label = new cEditorLabel(editorWindowEntity->GetHWND(), "Geometry",
            glm::vec2(offset * 3.0f, offset * 23.0f), glm::vec2(offset * 20.0f, offset * 7.0f)
        );
        editorWindowEntityGeometry.Textbox = new cEditorTextbox(editorWindowEntity->GetHWND(), "",
            glm::vec2(offset * 25.0f, offset * 23.0f), glm::vec2(offset * 85.0f, offset * 7.0f), types::K_FALSE, types::K_FALSE
        );
        editorWindowEntityDiffuseColor.Label = new cEditorLabel(editorWindowEntity->GetHWND(), "Color",
            glm::vec2(offset * 3.0f, offset * 32.0f), glm::vec2(offset * 20.0f, offset * 7.0f)
        );
        editorWindowEntityDiffuseColor.Textbox = new cEditorTextbox(editorWindowEntity->GetHWND(), "255;255;255;255",
            glm::vec2(offset * 25.0f, offset * 32.0f), glm::vec2(offset * 85.0f, offset * 7.0f), types::K_FALSE, types::K_FALSE
        );

        // Sound window
        editorWindowSound = new cEditorWindow(
            nullptr,
            "SoundWindow",
            "Sound",
            glm::vec2(0.0f),
            glm::vec2(offset * 120.0f, offset * 50.0f)
        );
        editorWindowSound->Show(types::K_FALSE);
        RemoveWindowSysmenu(editorWindowSound->GetHWND());

        editorWindowSoundOKButton = new cEditorButton(
            editorWindowSound->GetHWND(),
            "OK",
            glm::vec2(offset * 106.0f, offset * 30.0f),
            glm::vec2(offset * 7.0f, offset * 7.0f)
        );
        editorWindowSoundCloseButton = new cEditorButton(
            editorWindowSound->GetHWND(),
            "Close",
            glm::vec2(offset * 90.0f, offset * 30.0f),
            glm::vec2(offset * 15.0f, offset * 7.0f)
        );
        editorWindowSoundName.Label = new cEditorLabel(editorWindowSound->GetHWND(), "Name",
            glm::vec2(offset * 3.0f, offset * 5.0f), glm::vec2(offset * 20.0f, offset * 7.0f)
        );
        editorWindowSoundName.Textbox = new cEditorTextbox(editorWindowSound->GetHWND(), "",
            glm::vec2(offset * 25.0f, offset * 5.0f), glm::vec2(offset * 85.0f, offset * 7.0f), types::K_FALSE, types::K_FALSE
        );
        editorWindowSoundFile.Label = new cEditorLabel(editorWindowSound->GetHWND(), "File",
            glm::vec2(offset * 3.0f, offset * 14.0f), glm::vec2(offset * 20.0f, offset * 7.0f)
        );
        editorWindowSoundFile.Textbox = new cEditorTextbox(editorWindowSound->GetHWND(), "",
            glm::vec2(offset * 25.0f, offset * 14.0f), glm::vec2(offset * 85.0f, offset * 7.0f), types::K_FALSE, types::K_FALSE
        );

        // Script window
        editorWindowScript = new cEditorWindow(
            nullptr,
            "ScriptWindow",
            "Script",
            glm::vec2(0.0f),
            glm::vec2(offset * 120.0f, offset * 140.0f)
        );
        editorWindowScript->Show(types::K_FALSE);
        RemoveWindowSysmenu(editorWindowScript->GetHWND());

        editorWindowScriptOKButton = new cEditorButton(
            editorWindowScript->GetHWND(),
            "OK",
            glm::vec2(offset * 106.0f, offset * 120.0f),
            glm::vec2(offset * 7.0f, offset * 7.0f)
        );
        editorWindowScriptCloseButton = new cEditorButton(
            editorWindowScript->GetHWND(),
            "Close",
            glm::vec2(offset * 90.0f, offset * 120.0f),
            glm::vec2(offset * 15.0f, offset * 7.0f)
        );
        editorWindowScriptName.Label = new cEditorLabel(editorWindowScript->GetHWND(), "Name",
            glm::vec2(offset * 3.0f, offset * 5.0f), glm::vec2(offset * 20.0f, offset * 7.0f)
        );
        editorWindowScriptName.Textbox = new cEditorTextbox(editorWindowScript->GetHWND(), "",
            glm::vec2(offset * 25.0f, offset * 5.0f), glm::vec2(offset * 85.0f, offset * 7.0f), types::K_FALSE, types::K_FALSE
        );
        editorWindowScriptCode.Label = new cEditorLabel(editorWindowScript->GetHWND(), "Code",
            glm::vec2(offset * 3.0f, offset * 14.0f), glm::vec2(offset * 20.0f, offset * 7.0f)
        );
        editorWindowScriptCode.Textbox = new cEditorTextbox(editorWindowScript->GetHWND(), "",
            glm::vec2(offset * 3.0f, offset * 22.0f), glm::vec2(offset * 110.0f, offset * 90.0f), types::K_FALSE, types::K_TRUE
        );

        // Render window
        //this->SetWindowPosition(glm::vec2(assetWindowSize.x + (userInputManager->GetMonitorSize().x * 0.006f), userInputManager->GetMonitorSize().y * 0.041f));
        RemoveWindowSysmenu(this->GetWindowHWND());

        // Object window
        glm::vec2 objectWindowSize = glm::vec2(
            this->GetWindowSize().x / 1.86f, this->GetWindowSize().y / 3.91f
        );
        auto objectWindow = new cEditorWindow(
            nullptr,
            "ObjectWindow",
            "Object",
            glm::vec2(assetWindowSize.x, (this->GetWindowSize().y * 0.016f) + (this->GetWindowSize().y * 0.091f) + (this->GetWindowSize().y / 1.7f)),
            objectWindowSize
        );
        RemoveWindowSysmenu(objectWindow->GetHWND());

        auto objectPositionGroupbox = new cEditorGroupbox(
            objectWindow->GetHWND(),
            "Position",
            glm::vec2(offset),
            glm::vec2(objectWindowSize.x - offset * 140.0f, objectWindowSize.y - offset * 45.0f)
        );
        auto objectRotationGroupbox = new cEditorGroupbox(
            objectWindow->GetHWND(),
            "Rotation",
            glm::vec2(offset, offset * 15.0f),
            glm::vec2(objectWindowSize.x - offset * 140.0f, objectWindowSize.y - offset * 45.0f)
        );
        auto objectScaleGroupbox = new cEditorGroupbox(
            objectWindow->GetHWND(),
            "Scale",
            glm::vec2(offset, offset * 30.0f),
            glm::vec2(objectWindowSize.x - offset * 140.0f, objectWindowSize.y - offset * 45.0f)
        );
        auto objectComponentsGroupbox = new cEditorGroupbox(
            objectWindow->GetHWND(),
            "Components",
            glm::vec2((offset * 2.0f) + (objectWindowSize.x - (offset * 140.0f)), offset),
            glm::vec2(objectWindowSize.x - offset * 82.0f, objectWindowSize.y - offset * 13.0f)
        );

        editorPositionX.Label = new cEditorLabel(objectPositionGroupbox->GetHWND(), "X",
            glm::vec2(offset * 3.0f, offset * 5.0f), glm::vec2(offset * 3.0f, offset * 5.0f)
        );
        editorPositionX.Textbox = new cEditorTextbox(objectPositionGroupbox->GetHWND(), "0",
            glm::vec2(offset * 8.0f, offset * 5.0f), glm::vec2(offset * 18.0f, offset * 6.0f), types::K_FALSE, types::K_FALSE
        );
        editorPositionY.Label = new cEditorLabel(objectPositionGroupbox->GetHWND(), "Y",
            glm::vec2(offset * 26.0f, offset * 5.0f), glm::vec2(offset * 3.0f, offset * 5.0f)
        );
        editorPositionY.Textbox = new cEditorTextbox(objectPositionGroupbox->GetHWND(), "0",
            glm::vec2(offset * 31.0f, offset * 5.0f), glm::vec2(offset * 18.0f, offset * 6.0f), types::K_FALSE, types::K_FALSE
        );
        editorPositionZ.Label = new cEditorLabel(objectPositionGroupbox->GetHWND(), "Z",
            glm::vec2(offset * 49.0f, offset * 5.0f), glm::vec2(offset * 3.0f, offset * 5.0f)
        );
        editorPositionZ.Textbox = new cEditorTextbox(objectPositionGroupbox->GetHWND(), "0",
            glm::vec2(offset * 54.0f, offset * 5.0f), glm::vec2(offset * 18.0f, offset * 6.0f), types::K_FALSE, types::K_FALSE
        );
        editorRotationX.Label = new cEditorLabel(objectRotationGroupbox->GetHWND(), "X",
            glm::vec2(offset * 3.0f, offset * 5.0f), glm::vec2(offset * 3.0f, offset * 5.0f)
        );
        editorRotationX.Textbox = new cEditorTextbox(objectRotationGroupbox->GetHWND(), "0",
            glm::vec2(offset * 8.0f, offset * 5.0f), glm::vec2(offset * 18.0f, offset * 6.0f), types::K_FALSE, types::K_FALSE
        );
        editorRotationY.Label = new cEditorLabel(objectRotationGroupbox->GetHWND(), "Y",
            glm::vec2(offset * 26.0f, offset * 5.0f), glm::vec2(offset * 3.0f, offset * 5.0f)
        );
        editorRotationY.Textbox = new cEditorTextbox(objectRotationGroupbox->GetHWND(), "0",
            glm::vec2(offset * 31.0f, offset * 5.0f), glm::vec2(offset * 18.0f, offset * 6.0f), types::K_FALSE, types::K_FALSE
        );
        editorRotationZ.Label = new cEditorLabel(objectRotationGroupbox->GetHWND(), "Z",
            glm::vec2(offset * 49.0f, offset * 5.0f), glm::vec2(offset * 3.0f, offset * 5.0f)
        );
        editorRotationZ.Textbox = new cEditorTextbox(objectRotationGroupbox->GetHWND(), "0",
            glm::vec2(offset * 54.0f, offset * 5.0f), glm::vec2(offset * 18.0f, offset * 6.0f), types::K_FALSE, types::K_FALSE
        );
        editorScaleX.Label = new cEditorLabel(objectScaleGroupbox->GetHWND(), "X",
            glm::vec2(offset * 3.0f, offset * 5.0f), glm::vec2(offset * 3.0f, offset * 5.0f)
        );
        editorScaleX.Textbox = new cEditorTextbox(objectScaleGroupbox->GetHWND(), "1",
            glm::vec2(offset * 8.0f, offset * 5.0f), glm::vec2(offset * 18.0f, offset * 6.0f), types::K_FALSE, types::K_FALSE
        );
        editorScaleY.Label = new cEditorLabel(objectScaleGroupbox->GetHWND(), "Y",
            glm::vec2(offset * 26.0f, offset * 5.0f), glm::vec2(offset * 3.0f, offset * 5.0f)
        );
        editorScaleY.Textbox = new cEditorTextbox(objectScaleGroupbox->GetHWND(), "1",
            glm::vec2(offset * 31.0f, offset * 5.0f), glm::vec2(offset * 18.0f, offset * 6.0f), types::K_FALSE, types::K_FALSE
        );
        editorScaleZ.Label = new cEditorLabel(objectScaleGroupbox->GetHWND(), "Z",
            glm::vec2(offset * 49.0f, offset * 5.0f), glm::vec2(offset * 3.0f, offset * 5.0f)
        );
        editorScaleZ.Textbox = new cEditorTextbox(objectScaleGroupbox->GetHWND(), "1",
            glm::vec2(offset * 54.0f, offset * 5.0f), glm::vec2(offset * 18.0f, offset * 6.0f), types::K_FALSE, types::K_FALSE
        );

        editorLightScale.Label = new cEditorLabel(objectComponentsGroupbox->GetHWND(), "Scale",
            glm::vec2(offset * 29.0f, offset * 14.0f), glm::vec2(offset * 15.0f, offset * 5.0f)
        );
        editorLightScale.Textbox = new cEditorTextbox(objectComponentsGroupbox->GetHWND(), "1",
            glm::vec2(offset * 46.0f, offset * 14.0f), glm::vec2(offset * 8.0f, offset * 6.0f), types::K_FALSE, types::K_FALSE
        );
        editorLightScale.Textbox->SetReadonly(types::K_TRUE);
        editorLightColor.Label = new cEditorLabel(objectComponentsGroupbox->GetHWND(), "Color",
            glm::vec2(offset * 56.0f, offset * 14.0f), glm::vec2(offset * 15.0f, offset * 5.0f)
        );
        editorLightColor.Textbox = new cEditorTextbox(objectComponentsGroupbox->GetHWND(), "255;255;255;255",
            glm::vec2(offset * 73.0f, offset * 14.0f), glm::vec2(offset * 40.0f, offset * 6.0f), types::K_FALSE, types::K_FALSE
        );
        editorLightColor.Textbox->SetReadonly(types::K_TRUE);
        editorLightAttenuation.Label = new cEditorLabel(objectComponentsGroupbox->GetHWND(), "Attenuation",
            glm::vec2(offset * 29.0f, offset * 21.0f), glm::vec2(offset * 25.0f, offset * 5.0f)
        );
        editorLightAttenuation.Textbox = new cEditorTextbox(objectComponentsGroupbox->GetHWND(), "1.0;1.0;1.0",
            glm::vec2(offset * 56.0f, offset * 21.0f), glm::vec2(offset * 35.0f, offset * 6.0f), types::K_FALSE, types::K_FALSE
        );
        editorLightAttenuation.Textbox->SetReadonly(types::K_TRUE);

        editorScript.Label = new cEditorLabel(objectComponentsGroupbox->GetHWND(), "Name",
            glm::vec2(offset * 29.0f, offset * 28.0f), glm::vec2(offset * 15.0f, offset * 5.0f)
        );
        editorScript.Textbox = new cEditorTextbox(objectComponentsGroupbox->GetHWND(), "",
            glm::vec2(offset * 46.0f, offset * 28.0f), glm::vec2(offset * 40.0f, offset * 6.0f), types::K_FALSE, types::K_FALSE
        );

        ShowWindow(GetConsoleWindow(), SW_HIDE);
    }

    virtual void FrameUpdate() override final
    {
        static float deltaTime = 0.0f;
        static float lastTime = clock();

        // Left mouse button press
        types::boolean lmbPress = types::K_FALSE;
        static types::boolean lmbPressGlobal = types::K_FALSE;
        if (this->GetMouseKey((int)cApplication::eMouseButton::LEFT) == 1 && lmbPressGlobal == types::K_FALSE) {
            lmbPress = lmbPressGlobal = types::K_TRUE;
        } else if (this->GetMouseKey((int)cApplication::eMouseButton::LEFT) == 0) {
            lmbPressGlobal = types::K_FALSE;
        }

        // Right mouse button press
        types::boolean rmbPress = types::K_FALSE;
        static types::boolean rmbPressGlobal = types::K_FALSE;
        if (this->GetMouseKey((int)cApplication::eMouseButton::LEFT) == 1 && rmbPressGlobal == types::K_FALSE) {
            rmbPress = rmbPressGlobal = types::K_TRUE;
        } else if (this->GetMouseKey((int)cApplication::eMouseButton::LEFT) == 0) {
            rmbPressGlobal = types::K_FALSE;
        }
        
        EditorWindowAssetLogic();
        //EditorWindowObjectLogic(this, editorScene);
        //EditorWindowRenderEntityLogic(this, editorScene, editorCamera, lmbPress, rmbPress);
        //EditorUpdateEntityFields(editorScene);

        //physicsManager->Update();

        //cameraManager->Update(editorCamera, editorScene, K_FALSE, K_TRUE);
        //if (userInputManager->GetKey(GLFW_KEY_LEFT_ALT) == K_TRUE) {
        //    cameraManager->Update(editorCamera, editorScene, K_TRUE, K_FALSE);
        //}

        // Draw
        //renderManager->SetCamera(editorCamera);
        //renderManager->ClearRenderPasses(glm::vec4(1.0f), 1.0f);
        //for (auto vbGeometry : editorGeometriesToDraw) {
        //    renderManager->DrawGeometryOpaque(this, vbGeometry, editorScene);
        //}
        //renderManager->CompositeFinal();

        //userInputManager->SwapBuffers();

        //clock_t t = clock();

        //deltaTime = t - lastTime;
        //lastTime += deltaTime;
    }

    virtual void Finish() override final
    {
        //fontManager->Free();
    }

private:
    sVertexBufferGeometry* m_geomPlane;
    sVertexBufferGeometry* m_geomTaburet;
    //sArea* m_taburetTexture;

};

int main()
{
    sApplicationDescriptor appDesc;
    appDesc.WindowDesc.Title = "Render";
    appDesc.WindowDesc.Width = 640;
    appDesc.WindowDesc.Height = 480;
    appDesc.WindowDesc.IsFullscreen = types::K_FALSE;
    appDesc.MemoryPoolByteSize = 256 * 1024 * 1024;
    appDesc.MaxGameObjectCount = 50 * 50 * 50;
    appDesc.MaxRenderOpaqueInstanceCount = 50 * 50 * 50;

    MyApp app = MyApp(appDesc);
    app.Run();

    return 0;
}

void EditorUpdateTextboxTransform()//(sCTransform* transform)
{
    /*
    // Position
    std::string posXStr = std::to_string(transform->Position.x); posXStr.resize(5);
    editorPositionX.Textbox->SetText(posXStr);
    std::string posYStr = std::to_string(transform->Position.y); posYStr.resize(5);
    editorPositionY.Textbox->SetText(posYStr);
    std::string posZStr = std::to_string(transform->Position.z); posZStr.resize(5);
    editorPositionZ.Textbox->SetText(posZStr);
    editorPosition = glm::vec3(std::stof(posXStr), std::stof(posYStr), std::stof(posZStr));

    // Rotation
    std::string rotXStr = std::to_string(glm::degrees(transform->Rotation.x)); rotXStr.resize(5);
    editorRotationX.Textbox->SetText(rotXStr);
    std::string rotYStr = std::to_string(glm::degrees(transform->Rotation.y)); rotYStr.resize(5);
    editorRotationY.Textbox->SetText(rotYStr);
    std::string rotZStr = std::to_string(glm::degrees(transform->Rotation.z)); rotZStr.resize(5);
    editorRotationZ.Textbox->SetText(rotZStr);
    editorRotation = glm::vec3(std::stof(rotXStr), std::stof(rotYStr), std::stof(rotZStr));

    // Scale 
    std::string sclXStr = std::to_string(transform->Scale.x); sclXStr.resize(5);
    editorScaleX.Textbox->SetText(sclXStr);
    std::string sclYStr = std::to_string(transform->Scale.y); sclYStr.resize(5);
    editorScaleY.Textbox->SetText(sclYStr);
    std::string sclZStr = std::to_string(transform->Scale.z); sclZStr.resize(5);
    editorScaleZ.Textbox->SetText(sclZStr);
    editorScale = glm::vec3(std::stof(sclXStr), std::stof(sclYStr), std::stof(sclZStr));
    */
}

void EditorUpdateTextboxLight()//(sCLight* light)
{
    /*
    // Scale
    std::string scaleStr = std::to_string(light->Scale); scaleStr.resize(3);
    editorLightScale.Textbox->SetText(scaleStr);

    // Color
    std::string colorStr =
        std::to_string((realware::core::u32)(light->Color.x * 255.0f)) + std::string(";") +
        std::to_string((realware::core::u32)(light->Color.y * 255.0f)) + std::string(";") +
        std::to_string((realware::core::u32)(light->Color.z * 255.0f)) + std::string(";") +
        std::string("255");
    colorStr.resize(100);
    editorLightColor.Textbox->SetText(colorStr);

    // Attenuation
    std::string attenuationX = std::to_string(light->Attenuation.x); attenuationX.resize(3);
    std::string attenuationY = std::to_string(light->Attenuation.y); attenuationY.resize(3);
    std::string attenuationZ = std::to_string(light->Attenuation.z); attenuationZ.resize(3);
    std::string attenuationStr =
        attenuationX + std::string(";") +
        attenuationY + std::string(";") +
        attenuationZ;
    attenuationStr.resize(11);
    editorLightAttenuation.Textbox->SetText(attenuationStr);
    */
}

void EditorUpdateEntityFields()//(cScene* scene)
{
    /*if (editorSelectedEntity == 0) return;

    // Position
    std::string str1 = editorPositionX.Textbox->GetText(6);
    if (str1 == "") { str1 = "0.0"; }
    for (s32 i = 0; i < str1.size(); i++) { if (str1[1] < '.' || str1[i] > '9' || str1[i] == '/') { str1[i] = '0'; } }
    float textPosX = std::stof(str1);
    if (textPosX != editorPosition.x)
    {
        sCTransform* transform = scene->Get<sCTransform>(editorSelectedEntity);
        transform->Position.x = textPosX;
        editorPosition.x = textPosX;
    }
    std::string str2 = editorPositionY.Textbox->GetText(6);
    if (str2 == "") { str2 = "0.0"; }
    for (s32 i = 0; i < str2.size(); i++) { if (str2[1] < '.' || str2[i] > '9' || str2[i] == '/') { str2[i] = '0'; } }
    float textPosY = std::stof(str2);
    if (textPosY != editorPosition.y)
    {
        sCTransform* transform = scene->Get<sCTransform>(editorSelectedEntity);
        transform->Position.y = textPosY;
        editorPosition.y = textPosY;
    }
    std::string str3 = editorPositionZ.Textbox->GetText(6);
    if (str3 == "") { str3 = "0.0"; }
    for (s32 i = 0; i < str3.size(); i++) { if (str3[1] < '.' || str3[i] > '9' || str3[i] == '/') { str3[i] = '0'; } }
    float textPosZ = std::stof(str3);
    if (textPosZ != editorPosition.z)
    {
        sCTransform* transform = scene->Get<sCTransform>(editorSelectedEntity);
        transform->Position.z = textPosZ;
        editorPosition.z = textPosZ;
    }

    // Rotation
    std::string str4 = editorRotationX.Textbox->GetText(6);
    if (str4 == "") { str4 = "0.0"; }
    for (s32 i = 0; i < str4.size(); i++) { if (str4[1] < '.' || str4[i] > '9' || str4[i] == '/') { str4[i] = '0'; } }
    float textRotX = std::stof(str4);
    if (textRotX != editorRotation.x)
    {
        sCTransform* transform = scene->Get<sCTransform>(editorSelectedEntity);
        transform->Rotation.x = glm::radians(textRotX);
        editorRotation.x = textRotX;
    }
    std::string str5 = editorRotationY.Textbox->GetText(6);
    if (str5 == "") { str5 = "0.0"; }
    for (s32 i = 0; i < str5.size(); i++) { if (str5[1] < '.' || str5[i] > '9' || str5[i] == '/') { str5[i] = '0'; } }
    float textRotY = std::stof(str5);
    if (textRotY != editorRotation.y)
    {
        sCTransform* transform = scene->Get<sCTransform>(editorSelectedEntity);
        transform->Rotation.y = glm::radians(textRotY);
        editorRotation.y = textRotY;
    }
    std::string str6 = editorRotationZ.Textbox->GetText(6);
    if (str6 == "") { str6 = "0.0"; }
    for (s32 i = 0; i < str6.size(); i++) { if (str6[1] < '.' || str6[i] > '9' || str6[i] == '/') { str6[i] = '0'; } }
    float textRotZ = std::stof(str6);
    if (textRotZ != editorRotation.z)
    {
        sCTransform* transform = scene->Get<sCTransform>(editorSelectedEntity);
        transform->Rotation.z = glm::radians(textRotZ);
        editorRotation.z = textRotZ;
    }

    // Scale
    std::string str7 = editorScaleX.Textbox->GetText(6);
    if (str7 == "") { str7 = "0.0"; }
    for (s32 i = 0; i < str7.size(); i++) { if (str7[1] < '.' || str7[i] > '9' || str7[i] == '/') { str7[i] = '0'; } }
    float textSclX = std::stof(str7);
    if (textSclX != editorScale.x)
    {
        sCTransform* transform = scene->Get<sCTransform>(editorSelectedEntity);
        transform->Scale.x = textSclX;
        editorScale.x = textSclX;
    }
    std::string str8 = editorScaleY.Textbox->GetText(6);
    if (str8 == "") { str8 = "0.0"; }
    for (s32 i = 0; i < str8.size(); i++) { if (str8[1] < '.' || str8[i] > '9' || str8[i] == '/') { str8[i] = '0'; } }
    float textSclY = std::stof(str8);
    if (textSclY != editorScale.y)
    {
        sCTransform* transform = scene->Get<sCTransform>(editorSelectedEntity);
        transform->Scale.y = textSclY;
        editorScale.y = textSclY;
    }
    std::string str9 = editorScaleZ.Textbox->GetText(6);
    if (str9 == "") { str3 = "0.0"; }
    for (s32 i = 0; i < str9.size(); i++) { if (str9[1] < '.' || str9[i] > '9' || str9[i] == '/') { str9[i] = '0'; } }
    float textSclZ = std::stof(str9);
    if (textSclZ != editorScale.z)
    {
        sCTransform* transform = scene->Get<sCTransform>(editorSelectedEntity);
        transform->Scale.z = textSclZ;
        editorScale.z = textSclZ;
    }

    // Light
    std::string str10 = editorLightScale.Textbox->GetText(3);
    if (str10 == "") { str10 = "0.0"; }
    for (s32 i = 0; i < str10.size(); i++) { if (str10[1] < '.' || str10[i] > '9' || str10[i] == '/') { str10[i] = '0'; } }
    float textScale = std::stof(str10);
    sCLight* light = scene->Get<sCLight>(editorSelectedEntity);
    if (light != nullptr)
    {
        // Scale
        std::string colorChannels[4] = { "", "", "", "" }; usize colorChannelIndex = 0;
        std::string lightColorStr = editorLightColor.Textbox->GetText(100);
        for (auto c : lightColorStr)
        {
            if (c == ';') { colorChannelIndex += 1; }
            if (c < '0' || c > '9') { continue; }
            colorChannels[colorChannelIndex] += c;
        }

        // Attenuation
        std::string attenuationChannels[4] = { "", "", "", "" }; usize attenuationChannelIndex = 0;
        std::string lightAttenuationStr = editorLightAttenuation.Textbox->GetText(100);
        for (auto c : lightAttenuationStr)
        {
            if (c == ';') { attenuationChannelIndex += 1; }
            if ((c < '0' || c > '9') && c != '.') { continue; }
            attenuationChannels[attenuationChannelIndex] += c;
        }

        light->Color = glm::vec3(
            std::stoi(colorChannels[0]) / 255.0f,
            std::stoi(colorChannels[1]) / 255.0f,
            std::stoi(colorChannels[2]) / 255.0f
        );
        light->Attenuation = glm::vec3(
            (std::trunc(std::stof(attenuationChannels[0]) * 100.0f)) / 100.0f,
            (std::trunc(std::stof(attenuationChannels[1]) * 100.0f)) / 100.0f,
            (std::trunc(std::stof(attenuationChannels[2]) * 100.0f)) / 100.0f
        );
        light->Scale = textScale;

        renderManager->UpdateLights(editorApp, editorScene);
    }
    */
}

void EditorWindowAssetShowPopupmenu()//(realware::core::boolean rmbPress)
{
    /*if (rmbPress == realware::core::K_FALSE) {
        return;
    }

    HMENU hPopupMenu = CreatePopupMenu();

    POINT p = {};
    GetCursorPos(&p);

    AppendMenu(hPopupMenu, MF_STRING, 1, "New");
    AppendMenu(hPopupMenu, MF_STRING, 2, "Delete");
    AppendMenu(hPopupMenu, MF_SEPARATOR, 0, "");
    AppendMenu(hPopupMenu, MF_STRING, 3, "Use");

    TrackPopupMenu(
        hPopupMenu,
        TPM_LEFTALIGN | TPM_RIGHTBUTTON,
        p.x,
        p.y,
        0,
        editorWindowAssetListView->GetHWND(),
        nullptr
    );

    DestroyMenu(hPopupMenu);*/
}

void EditorWindowAssetDeleteItem(
    cApplication* app,
    //cScene* scene,
    const eAssetSelectedType& type,
    int assetIndex
)
{
    /*sAsset& asset = editorWindowAssetData[(int)type][assetIndex];
    if (type == eAssetSelectedType::ENTITY)
    {
        std::vector<entity> owners;
        if (asset.Components[0] != nullptr)
        {
            // Delete all related components
            scene->ForEach<sCMaterial>(
                app,
                [&asset, &owners, assetIndex](cApplication* app_, cScene* scene_, sCMaterial* material_)
                {
                    if (std::string((const char*)&scene_->Get<sCAssetName>(material_->Owner)->AssetName[0])
                        == asset.Name)
                    {
                        owners.push_back(material_->Owner);
                        return;
                    }
                }
            );

            // Delete texture
            textureManager->RemoveTexture(asset.Name + "Texture");
            delete asset.Components[0];
        }
        if (asset.Components[1] != nullptr)
        {
            // Delete all related components
            scene->ForEach<sCGeometry>(
                app,
                [&asset, &owners, assetIndex](cApplication* app_, cScene* scene_, sCGeometry* geometry_)
                {
                    if (std::string((const char*)&scene_->Get<sCAssetName>(geometry_->Owner)->AssetName[0])
                        == asset.Name)
                    {
                        owners.push_back(geometry_->Owner);
                        return;
                    }
                }
            );

            // Delete geometry
            delete asset.Components[1];
        }

        // Remove entity from scene
        for (auto owner : owners)
        {
            scene->Remove<sCAssetName>(owner);
            scene->Remove<sCTransform>(owner);
            scene->Remove<sCMaterial>(owner);
            scene->Remove<sCGeometry>(owner);
            scene->Remove<sCGeometryInfo>(owner);
            scene->RemoveEntity(scene->GetEntityName(owner));
        }
    }*/
}

void EditorAssetLoadData(eAssetSelectedType type, sAsset& asset)
{
    /*if (type == eAssetSelectedType::ENTITY)
    {
        if (asset.Components[0] == nullptr)
        {
            // Load texture
            auto texture = textureManager->CreateTexture(
                EditorGetExeFolder() + asset.Filenames[0],
                asset.Name + "Texture"
            );
            if (texture != nullptr)
            {
                asset.Components[0] = new sCMaterial();
                ((sCMaterial*)asset.Components[0])->Init(texture, asset.Color);
            }
            else
            {
                MessageBox(0, "Couldn't load texture", "Error", MB_ICONERROR);
            }
        }

        if (asset.Components[1] == nullptr)
        {
            // Load geometry
            auto geometry = renderManager->CreateModel(EditorGetExeFolder() + asset.Filenames[1]);
            if (geometry != nullptr)
            {
                auto vbGeometry = renderManager->AddGeometry(
                    geometry->Format,
                    geometry->VerticesByteSize,
                    geometry->Vertices,
                    geometry->IndicesByteSize,
                    geometry->Indices
                );
                renderManager->FreePrimitive(geometry);
                asset.Components[1] = new sCGeometry();
                ((sCGeometry*)asset.Components[1])->Geometry = vbGeometry;
                editorGeometriesToDraw.push_back(vbGeometry);
            }
            else
            {
                MessageBox(0, "Couldn't load model", "Error", MB_ICONERROR);
            }
        }
    }
    else if (type == eAssetSelectedType::SOUND)
    {
        if (asset.Components[0] == nullptr)
        {
            // Load sound
            auto sound = soundManager->LoadSound(
                (EditorGetExeFolder() + asset.Filenames[0]).c_str(),
                sSound::eFormat::WAV,
                asset.Name + "Texture"
            );
            if (sound != nullptr)
            {
                asset.Components[0] = new sCSound();
                ((sCSound*)asset.Components[0])->Sound = sound;
            }
            else
            {
                MessageBox(0, "Couldn't load sound", "Error", MB_ICONERROR);
            }
        }
    }*/
}

void EditorWindowEntityUpdate(int assetIndex)
{
    /*sAsset& asset = editorWindowAssetData[(int)editorWindowAssetSelectedType][assetIndex];
    std::string caption = "Entity : " + asset.Name;
    SetWindowText(editorWindowEntity->GetHWND(), caption.data());
    editorWindowEntityName.Textbox->SetText(asset.Name);
    editorWindowEntityTexture.Textbox->SetText(asset.Filenames[0]);
    editorWindowEntityGeometry.Textbox->SetText(asset.Filenames[1]);*/
}

void EditorWindowEntitySave()//(cApplication* app, cScene* scene, int assetIndex)
{
    /*sAsset& asset = editorWindowAssetData[(int)editorWindowAssetSelectedType][assetIndex];
    asset.Name = editorWindowEntityName.Textbox->GetText(100);
    asset.Filenames[0] = editorWindowEntityTexture.Textbox->GetText(100);
    asset.Filenames[1] = editorWindowEntityGeometry.Textbox->GetText(100);
    std::string channels[4] = { "", "", "", "" }; usize channelIndex = 0;
    for (auto c : editorWindowEntityDiffuseColor.Textbox->GetText(100))
    {
        if (c == ';') { channelIndex += 1; }
        if (c < '0' || c > '9') { continue; }
        channels[channelIndex] += c;
    }
    asset.Color = glm::vec4(
        std::stoi(channels[0]) / 255.0f,
        std::stoi(channels[1]) / 255.0f,
        std::stoi(channels[2]) / 255.0f,
        std::stoi(channels[3]) / 255.0f
    );

    EditorAssetLoadData(editorWindowAssetSelectedType, asset);

    // Update material for every entity
    if (asset.Components[0] != nullptr)
    {
        // Delete material
        textureManager->RemoveTexture(((sCMaterial*)asset.Components[0])->DiffuseTexture->Tag);
        delete asset.Components[0];
        asset.Components[0] = nullptr;

        EditorAssetLoadData(editorWindowAssetSelectedType, asset);

        ((sCMaterial*)asset.Components[0])->DiffuseColor = asset.Color;
        scene->ForEach<sCMaterial>(
            app,
            [&asset, assetIndex](cApplication* app_, cScene* scene_, sCMaterial* material_)
            {
                if (std::string((const char*)&scene_->Get<sCAssetName>(material_->Owner)->AssetName[0])
                    == asset.Name)
                    material_->DiffuseColor = asset.Color;
            }
        );
    }

    std::string caption = "Entity : " + asset.Name;
    SetWindowText(editorWindowEntity->GetHWND(), caption.data());
    ListView_SetItemText(editorWindowAssetListView->GetHWND(), assetIndex, 0, asset.Name.data());*/
}

void EditorWindowSoundUpdate(int assetIndex)
{
    sAsset& asset = editorWindowAssetData[(int)editorWindowAssetSelectedType][assetIndex];
    std::string caption = "Sound : " + asset.Name;
    SetWindowText(editorWindowSound->GetHWND(), caption.data());
    editorWindowSoundName.Textbox->SetText(asset.Name);
    editorWindowSoundFile.Textbox->SetText(asset.Filenames[0]);
}

void EditorWindowSoundSave()//(cApplication* app, cScene* scene, int assetIndex)
{
    /*sAsset& asset = editorWindowAssetData[(int)editorWindowAssetSelectedType][assetIndex];
    asset.Name = editorWindowSoundName.Textbox->GetText(100);
    asset.Filenames[0] = editorWindowSoundFile.Textbox->GetText(100);
    
    EditorAssetLoadData(editorWindowAssetSelectedType, asset);

    if (asset.Components[0] != nullptr)
    {
        // Delete material
        soundManager->RemoveSound(((sCSound*)asset.Components[0])->Sound->Tag);
        delete asset.Components[0];
        asset.Components[0] = nullptr;

        EditorAssetLoadData(editorWindowAssetSelectedType, asset);
    }

    std::string caption = "Sound : " + asset.Name;
    SetWindowText(editorWindowSound->GetHWND(), caption.data());
    ListView_SetItemText(editorWindowAssetListView->GetHWND(), assetIndex, 0, asset.Name.data());*/
}

void EditorWindowScriptUpdate(int assetIndex)
{
    sAsset& asset = editorWindowAssetData[(int)editorWindowAssetSelectedType][assetIndex];
    std::string caption = "Script : " + asset.Name;
    SetWindowText(editorWindowScript->GetHWND(), caption.data());
    editorWindowScriptName.Textbox->SetText(asset.Name);
    editorWindowScriptCode.Textbox->SetText(asset.Code);
}

void EditorWindowScriptSave()//(cApplication* app, cScene* scene, int assetIndex)
{
    /*sAsset& asset = editorWindowAssetData[(int)editorWindowAssetSelectedType][assetIndex];
    asset.Name = editorWindowScriptName.Textbox->GetText(100);
    asset.Code = editorWindowScriptCode.Textbox->GetText(100);

    std::string caption = "Script : " + asset.Name;
    SetWindowText(editorWindowScript->GetHWND(), caption.data());
    ListView_SetItemText(editorWindowAssetListView->GetHWND(), assetIndex, 0, asset.Name.data());*/
}

void EditorWindowRenderEntityLogic(
    cApplication* app
    //cScene* scene,
    //entity camera,
    //realware::core::boolean lmbPress,
    //realware::core::boolean rmbPress
)
{
    // Create/transform entity and change selection mode
    /*switch (editorSelectMode)
    {
        case eSelectMode::NONE:
        {
            // Ray triangle intersection
            realware::core::entity resultEntity = 0;
            realware::core::boolean resultBool = K_FALSE;
            glm::vec3 result = glm::vec3(0.0f);
            cRayHit ray(
                app,
                scene,
                {},
                {},
                camera,
                userInputManager->GetCursorPosition(),
                userInputManager->GetWindowSize(),
                resultEntity,
                resultBool,
                result
            );

            break;
        }

        case eSelectMode::CREATE:
        {
            // Ray triangle intersection
            realware::core::entity resultEntity = 0;
            realware::core::boolean resultBool = K_FALSE;
            glm::vec3 result = glm::vec3(0.0f);
            cRayHit ray(
                app,
                scene,
                {},
                {},
                camera,
                userInputManager->GetCursorPosition(),
                userInputManager->GetWindowSize(),
                resultEntity,
                resultBool,
                result
            );

            if (rmbPress == K_TRUE)
            {
                if (editorUsedAssetIndex != -1 &&
                    editorWindowAssetSelectedType == eAssetSelectedType::ENTITY)
                {
                    sAsset& asset = editorWindowAssetData[eAssetSelectedType::ENTITY][editorUsedAssetIndex];

                    // Create entity
                    auto entity = scene->CreateEntity(asset.Name + std::to_string(editorUniqueID++));
                    sCAssetName* entityAssetName = scene->Add<sCAssetName>(entity);
                    memcpy(&entityAssetName->AssetName[0], asset.Name.data(), asset.Name.size());
                    sCGeometryInfo* entityGeometryInfo = scene->Add<sCGeometryInfo>(entity);
                    entityGeometryInfo->IsVisible = K_TRUE;
                    entityGeometryInfo->IsOpaque = K_TRUE;
                    sCMaterial* entityMaterial = scene->Add<sCMaterial>(entity);
                    if (entityMaterial != nullptr && (sCMaterial*)asset.Components[0] != nullptr)
                    {
                        entityMaterial->DiffuseColor = ((sCMaterial*)asset.Components[0])->DiffuseColor;
                        entityMaterial->HighlightColor = glm::vec4(1.0f);
                        entityMaterial->DiffuseTexture = ((sCMaterial*)asset.Components[0])->DiffuseTexture;
                    }
                    sCGeometry* entityGeometry = scene->Add<sCGeometry>(entity);
                    entityGeometry->Geometry = ((sCGeometry*)asset.Components[1])->Geometry;
                    sCTransform* entityTransform = scene->Add<sCTransform>(entity);
                    entityTransform->Position = result;
                    entityTransform->Rotation = glm::vec3(0.0f);
                    entityTransform->Scale = glm::vec3(1.0f);
                }
            }

            if (lmbPress == K_TRUE && resultBool == K_TRUE)
            {
                editorSelectMode = eSelectMode::TRANSFORM;
                editorSelectedEntity = resultEntity;

                if (editorSelectedEntity > 0)
                {
                    sCMaterial* material = scene->Get<sCMaterial>(editorSelectedEntity);
                    if (material != nullptr) material->HighlightColor = glm::vec4(1.75f);
                    sCTransform* transform = scene->Get<sCTransform>(editorSelectedEntity);
                    if (transform != nullptr) EditorUpdateTextboxTransform(transform);
                    sCLight* light = scene->Get<sCLight>(editorSelectedEntity);
                    if (light != nullptr)
                    {
                        editorIsLight->SetCheck(1);
                        EditorUpdateTextboxLight(light);
                    }
                    else
                    {
                        editorIsLight->SetCheck(0);
                    }
                }
            }

            break;
        }

        case eSelectMode::TRANSFORM:
        {
            // Ray triangle intersection
            realware::core::entity resultEntity = 0;
            realware::core::boolean resultBool = K_FALSE;
            glm::vec3 result = glm::vec3(0.0f);

            if (lmbPress == K_TRUE)
            {
                cRayHit ray(
                    app,
                    scene,
                    {},
                    {},
                    camera,
                    userInputManager->GetCursorPosition(),
                    userInputManager->GetWindowSize(),
                    resultEntity,
                    resultBool,
                    result
                );

                if (resultBool == K_TRUE && resultEntity != editorSelectedEntity)
                {
                    // Select another entity
                    sCMaterial* material = nullptr;

                    if (editorSelectedEntity > 0)
                    {
                        material = scene->Get<sCMaterial>(editorSelectedEntity);
                        if (material != nullptr) material->HighlightColor = glm::vec4(1.0f);
                    }

                    editorSelectedEntity = resultEntity;

                    if (editorSelectedEntity > 0)
                    {
                        material = scene->Get<sCMaterial>(editorSelectedEntity);
                        if (material != nullptr) material->HighlightColor = glm::vec4(1.75f);
                        sCTransform* transform = scene->Get<sCTransform>(editorSelectedEntity);
                        if (transform != nullptr) EditorUpdateTextboxTransform(transform);
                        sCLight* light = scene->Get<sCLight>(editorSelectedEntity);
                        if (light != nullptr)
                        {
                            editorIsLight->SetCheck(1);
                            EditorUpdateTextboxLight(light);
                        }
                        else
                        {
                            editorIsLight->SetCheck(0);
                        }
                    }
                }
                else if (resultBool == K_FALSE || resultEntity == editorSelectedEntity)
                {
                    // Deselect entity
                    if (editorSelectedEntity > 0)
                    {
                        sCMaterial* material = scene->Get<sCMaterial>(editorSelectedEntity);
                        if (material != nullptr) material->HighlightColor = glm::vec4(1.0f);
                    }

                    editorSelectedEntity = 0;

                    editorPositionX.Textbox->SetText("0.0");
                    editorPositionY.Textbox->SetText("0.0");
                    editorPositionZ.Textbox->SetText("0.0");
                    editorRotationX.Textbox->SetText("0.0");
                    editorRotationY.Textbox->SetText("0.0");
                    editorRotationZ.Textbox->SetText("0.0");
                    editorScaleX.Textbox->SetText("0.0");
                    editorScaleY.Textbox->SetText("0.0");
                    editorScaleZ.Textbox->SetText("0.0");
                }
            }

            if (userInputManager->GetMouseKey(GLFW_MOUSE_BUTTON_RIGHT) == mUserInput::eButtonState::PRESSED &&
                editorSelectedEntity != 0)
            {
                cRayHit ray(
                    app,
                    scene,
                    {},
                    { editorSelectedEntity },
                    camera,
                    userInputManager->GetCursorPosition(),
                    userInputManager->GetWindowSize(),
                    resultEntity,
                    resultBool,
                    result
                );

                if (resultBool == K_TRUE)
                {
                    // Move entity
                    sCTransform* transform = scene->Get<sCTransform>(editorSelectedEntity);
                    if (transform != nullptr) transform->Position = result;
                    EditorUpdateTextboxTransform(transform);
                }
            }

            break;
        }
    }

    // 'V' button press
    realware::core::boolean vButtonPress = K_FALSE;
    static realware::core::boolean vButtonPressGlobal = K_FALSE;
    if (userInputManager->GetKey(GLFW_KEY_V) == K_TRUE && vButtonPressGlobal == K_FALSE) {
        vButtonPress = vButtonPressGlobal = K_TRUE;
    } else if (userInputManager->GetKey(GLFW_KEY_V) == K_FALSE) {
        vButtonPressGlobal = K_FALSE;
    }

    // Ctrl+C
    if (userInputManager->GetKey(GLFW_KEY_LEFT_CONTROL) == K_TRUE &&
        userInputManager->GetKey(GLFW_KEY_C) == K_TRUE &&
        editorSelectedEntity > 0 &&
        editorSelectMode == eSelectMode::TRANSFORM)
    {
        editorCopyEntity = editorSelectedEntity;
    }

    // Ctrl+V
    if (userInputManager->GetKey(GLFW_KEY_LEFT_CONTROL) == K_TRUE &&
        vButtonPress == K_TRUE &&
        editorCopyEntity > 0 &&
        editorSelectMode == eSelectMode::TRANSFORM)
    {
        // Create entity
        sCAssetName* originalAssetName = scene->Get<sCAssetName>(editorCopyEntity);
        auto entity = scene->CreateEntity(std::string((const char*)&originalAssetName->AssetName[0]) + std::to_string(editorUniqueID++));
        sCAssetName* entityAssetName = scene->Add<sCAssetName>(entity);
        entityAssetName->Owner = entity;
        memcpy(&entityAssetName->AssetName[0], &originalAssetName->AssetName[0], 256);
        sCGeometryInfo* entityGeometryInfo = scene->Add<sCGeometryInfo>(entity);
        entityGeometryInfo->Owner = entity;
        sCGeometryInfo* originalGeometryInfo = scene->Get<sCGeometryInfo>(editorCopyEntity);
        entityGeometryInfo->IsVisible = K_TRUE;
        entityGeometryInfo->IsOpaque = K_TRUE;
        sCMaterial* entityMaterial = scene->Add<sCMaterial>(entity);
        entityMaterial->Owner = entity;
        sCMaterial* originalMaterial = scene->Get<sCMaterial>(editorCopyEntity);
        entityMaterial->DiffuseColor = originalMaterial->DiffuseColor;
        entityMaterial->HighlightColor = glm::vec4(1.0f);
        entityMaterial->DiffuseTexture = originalMaterial->DiffuseTexture;
        sCGeometry* entityGeometry = scene->Add<sCGeometry>(entity);
        entityGeometry->Owner = entity;
        sCGeometry* originalGeometry = scene->Get<sCGeometry>(editorCopyEntity);
        entityGeometry->Geometry = originalGeometry->Geometry;
        sCTransform* entityTransform = scene->Add<sCTransform>(entity);
        entityTransform->Owner = entity;
        sCTransform* originalTransform = scene->Get<sCTransform>(editorCopyEntity);
        entityTransform->Position = originalTransform->Position;
        entityTransform->Rotation = originalTransform->Rotation;
        entityTransform->Scale = originalTransform->Scale;
    }

    // Ctrl+X
    if (userInputManager->GetKey(GLFW_KEY_LEFT_CONTROL) == K_TRUE &&
        userInputManager->GetKey(GLFW_KEY_X) == K_TRUE &&
        editorSelectedEntity > 0 &&
        editorSelectMode == eSelectMode::TRANSFORM)
    {
        if (scene->Get<sCAssetName>(editorSelectedEntity) != nullptr) scene->Remove<sCAssetName>(editorSelectedEntity);
        if (scene->Get<sCGeometryInfo>(editorSelectedEntity) != nullptr) scene->Remove<sCGeometryInfo>(editorSelectedEntity);
        if (scene->Get<sCMaterial>(editorSelectedEntity) != nullptr) scene->Remove<sCMaterial>(editorSelectedEntity);
        if (scene->Get<sCGeometry>(editorSelectedEntity) != nullptr) scene->Remove<sCGeometry>(editorSelectedEntity);
        if (scene->Get<sCTransform>(editorSelectedEntity) != nullptr) scene->Remove<sCTransform>(editorSelectedEntity);
        scene->RemoveEntity(scene->GetEntityName(editorSelectedEntity));

        editorSelectedEntity = 0;
    }*/
}

void EditorWindowObjectLogic()//(cApplication* app, cScene* scene)
{
    /*if (editorSelectedEntity <= 0) return;
    
    // Change 'Is visible' checkbox
    if (editorIsVisible->GetCheck())
    {
        scene->Get<sCGeometryInfo>(editorSelectedEntity)->IsVisible = K_TRUE;
    }
    else
    {
        scene->Get<sCGeometryInfo>(editorSelectedEntity)->IsVisible = K_TRUE;//+ 1;
    }

    // Change 'Is light' checkbox
    if (editorIsLight->GetCheck())
    {
        if (scene->Get<sCLight>(editorSelectedEntity) == nullptr)
            scene->Add<sCLight>(editorSelectedEntity);
        editorLightScale.Textbox->SetReadonly(K_TRUE);
        editorLightColor.Textbox->SetReadonly(K_TRUE);
        editorLightAttenuation.Textbox->SetReadonly(K_TRUE);
    }
    else
    {
        if (scene->Get<sCLight>(editorSelectedEntity) != nullptr)
            scene->Remove<sCLight>(editorSelectedEntity);
        editorLightScale.Textbox->SetReadonly(K_FALSE);
        editorLightColor.Textbox->SetReadonly(K_FALSE);
        editorLightAttenuation.Textbox->SetReadonly(K_FALSE);
    }*/
}

void EditorWindowAssetLogic()
{
    // Do search
    /*std::string text = editorWindowAssetSearch.Textbox->GetText(100);

    if (text == "") return;

    for (s32 i = 0; i < editorWindowAssetData[(int)editorWindowAssetSelectedType].size(); i++)
    {
        const sAsset& asset = editorWindowAssetData[(int)editorWindowAssetSelectedType][i];
        if (asset.Name == text)
        {
            editorWindowAssetListView->SetSelected(i);
        }
    }*/
}

void EditorNewMap()//(cApplication* app, cScene* scene)
{
    /*if (MessageBox(0, "Current map will be completely deleted. Are you sure?", "Warning", MB_ICONWARNING | MB_YESNOCANCEL) == IDYES)
    {
        scene->ForEachEntity(
            app,
            [](cApplication* app_, cScene* scene_, const std::string& id_, entity entity_)
            {
                if (scene_->Get<sCAssetName>(entity_) != nullptr)
                    scene_->Remove<sCAssetName>(entity_);
                scene_->Remove<sCGeometryInfo>(entity_);
                scene_->Remove<sCMaterial>(entity_);
                scene_->Remove<sCGeometry>(entity_);
                if (scene_->Get<sCTransform>(entity_) != nullptr &&
                    scene_->Get<sCAssetName>(entity_) != nullptr) scene_->Remove<sCTransform>(entity_);
            }
        );
        scene->RemoveEntities({ editorCamera, editorPhysicsScene });
        editorSelectedEntity = 0;
    }*/
}

void EditorOpenMap()//(cApplication* app, cScene* scene, const std::string& filename)
{
    //cMapPluginLoader loader;
    //loader.LoadMap(filename, app, scene);
}

void EditorSaveMap()//(cApplication* app, cScene* scene, const std::string& filename)
{
    //cMapPluginLoader loader;
    //loader.SaveMap(filename, app, scene, 65536, { editorCamera, editorPhysicsScene });
}

void EditorNewPlugin()//(cApplication* app, cScene* scene)
{
    /*if (MessageBox(0, "Current plugin will be completely deleted. Are you sure?", "Warning", MB_ICONWARNING | MB_YESNOCANCEL) == IDYES)
    {
        for (realware::core::s32 i = 0; i < eAssetSelectedType::_COUNT; i++)
        {
            for (realware::core::s32 j = 0; j < editorWindowAssetData[i].size(); j++)
            {
                EditorWindowAssetDeleteItem(app, scene, (eAssetSelectedType)i, j);
            }

            editorWindowAssetData[i].clear();
            editorWindowAssetData[i].shrink_to_fit();
        }
        ListView_DeleteAllItems(editorWindowAssetListView->GetHWND());
        editorSelectedEntity = 0;
    }*/
}

void EditorOpenPlugin()//(cApplication* app, cScene* scene, const std::string& filename)
{
    //cMapPluginLoader loader;
    //loader.LoadPlugin(filename, app, scene);
}

void EditorSavePlugin()//(cApplication* app, cScene* scene, const std::string& filename)
{
    //cMapPluginLoader loader;
    //loader.SavePlugin(filename, app, scene, 65536);
}

std::string EditorGetExeFolder()
{
    char path[MAX_PATH] = {};
    GetModuleFileNameA(NULL, &path[0], MAX_PATH);

    types::s32 i;
    for (i = MAX_PATH - 1; i > 0; i--)
    {
        if (path[i] == '/' || path[i] == '\\') break;
    }

    return std::string(&path[0]).substr(0, i) + "\\";
}