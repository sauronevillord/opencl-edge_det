#include "main_window.h"

using namespace std;

int main(int argc, char *argv[]){
    auto app = Gtk::Application::create("org.gtkmm.example");

    return app->make_window_and_run<MainWindow>(argc, argv);
}