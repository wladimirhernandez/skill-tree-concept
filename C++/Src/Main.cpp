#include <NsCore/Noesis.h>
#include <NsCore/RegisterComponent.h>
#include <NsCore/EnumConverter.h>
#include <NsApp/EmbeddedXamlProvider.h>
#include <NsApp/EmbeddedFontProvider.h>
#include <NsApp/ApplicationLauncher.h>
#include <NsApp/Window.h>
#include <NsApp/EntryPoint.h>
#include <NsGui/KeyBinding.h>
#include <NsGui/UICollection.h>

#include "ViewModel.h"

// Assets embedded by bin2h (see CustomBuild entries in the vcxproj)
#include "Resources.xaml.bin.h"
#include "MainPage.xaml.bin.h"
#include "Alegreya.ttf.bin.h"


using namespace Noesis;
using namespace NoesisApp;


////////////////////////////////////////////////////////////////////////////////////////////////////
class AppLauncher final: public ApplicationLauncher
{
private:
    Noesis::Ptr<XamlProvider> GetXamlProvider() const override
    {
        EmbeddedXaml xamls[] =
        {
            { "Resources.xaml", Resources_xaml },
            { "MainPage.xaml", MainPage_xaml }
        };

        return *new EmbeddedXamlProvider(xamls);
    }

    Noesis::Ptr<FontProvider> GetFontProvider() const override
    {
        EmbeddedFont fonts[] =
        {
            { "Fonts", Alegreya_ttf }
        };

        return *new EmbeddedFontProvider(fonts);
    }

    Uri GetResources(bool /*darkTheme*/) const override
    {
        return "Resources.xaml";
    }

    void OnAppStartUp() override
    {
        Ptr<SkillTree::ViewModel> vm = MakePtr<SkillTree::ViewModel>();

        Window* window = GetMainWindow();
        window->SetDataContext(vm);

        // ENTER applies the selection, ESC discards it (or closes when nothing is pending).
        // Bound on the window, the root of the tree, so they work whatever has focus.
        InputBindingCollection* bindings = window->GetInputBindings();
        bindings->Add(MakePtr<KeyBinding>(vm->GetApplyCommand(), Key_Enter, ModifierKeys_None));
        bindings->Add(MakePtr<KeyBinding>(vm->GetCloseCommand(), Key_Escape, ModifierKeys_None));
        window->Focus();
    }

    void RegisterComponents() const override
    {
        // Lets DataTrigger Value="Owned" etc. compare against the enum properties
        RegisterComponent<EnumConverter<SkillTree::NodeState>>();
        RegisterComponent<EnumConverter<SkillTree::LinkState>>();
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
int NsMain(int argc, char** argv)
{
    AppLauncher launcher;
    launcher.SetArguments(argc, argv);
    launcher.SetStartupUri("MainPage.xaml");
    return launcher.Run();
}
