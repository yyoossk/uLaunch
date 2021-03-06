#include <ui/ui_QuickMenu.hpp>
#include <ui/ui_MenuApplication.hpp>
#include <cfg/cfg_Config.hpp>
#include <am_DaemonMessages.hpp>

extern cfg::Theme g_ul_theme;
extern ui::MenuApplication::Ref g_menu_app_instance;

namespace ui
{
    static Mutex g_quick_menu_home_lock = EmptyMutex;
    static bool g_quick_menu_home_pressed = false;

    static void QuickMenuOnHomeButtonDetection()
    {
        mutexLock(&g_quick_menu_home_lock);
        g_quick_menu_home_pressed = true;
        mutexUnlock(&g_quick_menu_home_lock);
    }

    QuickMenu::QuickMenu(const std::string &main_icon)
    {
        this->on = false;
        this->bgalpha = 0;

        pu::ui::Color textclr = pu::ui::Color::FromHex(g_menu_app_instance->GetUIConfigValue<std::string>("text_color", "#e1e1e1ff"));
        pu::ui::Color menufocusclr = pu::ui::Color::FromHex(g_menu_app_instance->GetUIConfigValue<std::string>("menu_focus_color", "#5ebcffff"));
        pu::ui::Color menubgclr = pu::ui::Color::FromHex(g_menu_app_instance->GetUIConfigValue<std::string>("menu_bg_color", "#0094ffff"));

        this->options_menu = pu::ui::elm::Menu::New(200, 115, 880, menubgclr, 60, 8);
        this->options_menu->SetOnFocusColor(menufocusclr);
        g_menu_app_instance->ApplyConfigForElement("quick_menu", "quick_menu_item", this->options_menu);
        
        auto opt_item = pu::ui::elm::MenuItem::New("Help & information");
        opt_item->SetIcon(cfg::GetAssetByTheme(g_ul_theme, "ui/HelpIcon.png"));
        opt_item->AddOnClick(&actions::ShowHelpDialog);
        opt_item->SetColor(textclr);
        this->options_menu->AddItem(opt_item);

        opt_item = pu::ui::elm::MenuItem::New("Power options");
        opt_item->SetIcon(cfg::GetAssetByTheme(g_ul_theme, "ui/PowerIcon.png"));
        opt_item->AddOnClick(&actions::ShowPowerDialog);
        opt_item->SetColor(textclr);
        this->options_menu->AddItem(opt_item);

        opt_item = pu::ui::elm::MenuItem::New("Controller options");
        opt_item->SetIcon(cfg::GetAssetByTheme(g_ul_theme, "ui/ControllerIcon.png"));
        opt_item->AddOnClick(&actions::ShowControllerSupport);
        opt_item->SetColor(textclr);
        this->options_menu->AddItem(opt_item);

        opt_item = pu::ui::elm::MenuItem::New("Open album");
        opt_item->SetIcon(cfg::GetAssetByTheme(g_ul_theme, "ui/AlbumIcon.png"));
        opt_item->AddOnClick(&actions::ShowAlbumApplet);
        opt_item->SetColor(textclr);
        this->options_menu->AddItem(opt_item);

        opt_item = pu::ui::elm::MenuItem::New("Open web-page");
        opt_item->SetIcon(cfg::GetAssetByTheme(g_ul_theme, "ui/WebIcon.png"));
        opt_item->AddOnClick(&actions::ShowWebPage);
        opt_item->SetColor(textclr);
        this->options_menu->AddItem(opt_item);

        opt_item = pu::ui::elm::MenuItem::New("User menu");
        opt_item->SetIcon(cfg::GetAssetByTheme(g_ul_theme, "ui/UserIcon.png"));
        opt_item->AddOnClick(&actions::ShowUserMenu);
        opt_item->SetColor(textclr);
        this->options_menu->AddItem(opt_item);

        opt_item = pu::ui::elm::MenuItem::New("Themes menu");
        opt_item->SetIcon(cfg::GetAssetByTheme(g_ul_theme, "ui/ThemesIcon.png"));
        opt_item->AddOnClick(&actions::ShowThemesMenu);
        opt_item->SetColor(textclr);
        this->options_menu->AddItem(opt_item);

        opt_item = pu::ui::elm::MenuItem::New("Settings menu");
        opt_item->SetIcon(cfg::GetAssetByTheme(g_ul_theme, "ui/SettingsIcon.png"));
        opt_item->AddOnClick(&actions::ShowSettingsMenu);
        opt_item->SetColor(textclr);
        this->options_menu->AddItem(opt_item);
    }

    void QuickMenu::RegisterHomeButtonDetection()
    {
        am::RegisterOnMessageDetect(&QuickMenuOnHomeButtonDetection, am::MenuMessage::HomeRequest);
    }

    s32 QuickMenu::GetX()
    {
        return 0;
    }

    s32 QuickMenu::GetY()
    {
        return 0;
    }

    s32 QuickMenu::GetWidth()
    {
        return 1280;
    }

    s32 QuickMenu::GetHeight()
    {
        return 720;
    }

    void QuickMenu::Toggle()
    {
        this->on = !this->on;
    }
    
    bool QuickMenu::IsOn()
    {
        return this->on && (this->bgalpha > 0);
    }

    void QuickMenu::OnRender(pu::ui::render::Renderer::Ref &Drawer, s32 X, s32 Y)
    {
        if(!this->on)
        {
            if(this->bgalpha > 0)
            {
                this->bgalpha -= 20;
                if(this->bgalpha < 0) this->bgalpha = 0;
            }
        }
        else
        {
            if(this->bgalpha < 220)
            {
                this->bgalpha += 20;
                if(this->bgalpha > 220) this->bgalpha = 220;
            }
        }
        this->options_menu->SetVisible(this->bgalpha != 0);

        Drawer->RenderRectangleFill({ 50, 50, 50, (u8)this->bgalpha }, 0, 0, 1280, 720);

        if(this->bgalpha > 0)
        {
            if(this->bgalpha < 220) Drawer->SetBaseRenderAlpha((u8)this->bgalpha);
            this->options_menu->OnRender(Drawer, this->options_menu->GetProcessedX(), this->options_menu->GetProcessedY());
            if(this->bgalpha < 220) Drawer->UnsetBaseRenderAlpha();
        }
    }

    void QuickMenu::OnInput(u64 Down, u64 Up, u64 Held, pu::ui::Touch Pos)
    {
        if(this->on) this->options_menu->OnInput(Down, Up, Held, Pos);

        if((Down & KEY_L) || (Down & KEY_R) || (Down & KEY_ZL) || (Down & KEY_ZR)) this->Toggle();
        else if((Down & KEY_B) || (Down & KEY_A))
        {
            // B only valid for toggling off
            // A = something selected in the menu :P
            if(this->on) this->Toggle();
        }
        else
        {
            if(this->on)
            {
                mutexLock(&g_quick_menu_home_lock);
                auto home_pressed = g_quick_menu_home_pressed;
                g_quick_menu_home_pressed = false;
                mutexUnlock(&g_quick_menu_home_lock);
                if(home_pressed) this->Toggle();
            }
        }
    }
}