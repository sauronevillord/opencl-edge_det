#include <gtkmm.h>
// #include <gdkmm/pixbuf.h>

// class ImgCanvas : public Gtk::DrawingArea
// {
// public:
//     ImgCanvas(Glib::RefPtr<Gdk::Pixbuf> img);
//     ~ImgCanvas() override;

//     Glib::RefPtr<Gdk::Pixbuf> img;

// protected:
//     void on_draw(const Cairo::RefPtr<Cairo::Context> &cr, int width, int height);
// };

class MainWindow : public Gtk::Window
{
public:
    MainWindow();
    ~MainWindow() override;

protected:
    void on_button_clicked();
    void hide_dialog();
    void on_browse_clicked();

    void on_file_dialog_response(int response_id, Gtk::FileChooserDialog *dialog);

    Gtk::Box v_box;
    Gtk::Box h_box1;
    Gtk::Box h_box2;

    Gtk::Label input_label, output_label;
    Gtk::Entry input_entry, output_entry;

    Gtk::Button browse_button;
    Gtk::Button start_button;

    // ImgCanvas in_canvas;
    // ImgCanvas out_canvas;

    std::unique_ptr<Gtk::MessageDialog> err_dialog;
};