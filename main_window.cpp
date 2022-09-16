#include "main_window.h"

#include <CL/cl.h>
#include <vector>
#include <iostream>
#include <CImg.h>

using namespace std;
using namespace cimg_library;

MainWindow::MainWindow():
// in_canvas(NULL),
// out_canvas(NULL),
start_button("Start"),
browse_button("..."),
input_label("Percorso img di input: "),
output_label("Percorso img di output: ")
{
    set_title("Convoluzione OpenCL");
    set_default_size(800, 200);

    v_box.set_orientation(Gtk::Orientation::VERTICAL);
    v_box.set_margin(10);

    h_box1.set_orientation(Gtk::Orientation::HORIZONTAL);
    h_box1.set_margin(10);

    h_box2.set_orientation(Gtk::Orientation::HORIZONTAL);
    h_box2.set_margin(10);

    set_child(v_box);

    start_button.signal_clicked().connect(sigc::bind(
        sigc::mem_fun(*this, &MainWindow::on_button_clicked)
    ));

    browse_button.signal_clicked().connect(sigc::bind(
        sigc::mem_fun(*this, &MainWindow::on_browse_clicked)
    ));
    browse_button.set_hexpand(false);

    input_entry.set_hexpand();
    input_entry.set_margin_start(10);
    input_entry.set_margin_end(10);

    output_entry.set_hexpand();
    output_entry.set_margin_start(10);
    output_entry.set_margin_end(10);

    h_box1.append(input_label);
    h_box1.append(input_entry);
    h_box1.append(browse_button);
    // h_box1.append(in_canvas);

    h_box2.append(output_label);
    h_box2.append(output_entry);
    // h_box2.append(out_canvas);

    v_box.append(h_box1);
    v_box.append(h_box2);
    v_box.append(start_button);
}

MainWindow::~MainWindow(){}

void MainWindow::on_browse_clicked(){
    auto dialog = new Gtk::FileChooserDialog("Scegli un immagine di input",
        Gtk::FileChooser::Action::OPEN
    );
    dialog->set_transient_for(*this);
    dialog->set_modal(true);
    dialog->signal_response().connect(sigc::bind(
        sigc::mem_fun(*this, &MainWindow::on_file_dialog_response),
        dialog
    ));

    dialog->add_button("Annulla", Gtk::ResponseType::CANCEL);
    dialog->add_button("Apri", Gtk::ResponseType::OK);

    auto filter_img = Gtk::FileFilter::create();
    filter_img->set_name("Image files");
    filter_img->add_mime_type("image/png");
    filter_img->add_mime_type("image/jpeg");
    filter_img->add_mime_type("image/bmp");
    filter_img->add_mime_type("image/x-icon");
    filter_img->add_mime_type("image/pjpeg");
    filter_img->add_mime_type("image/ppm");
    dialog->add_filter(filter_img);

    dialog->show();
}

void MainWindow::on_file_dialog_response(int response_id, Gtk::FileChooserDialog *dialog){
    switch (response_id)
    {
    case Gtk::ResponseType::OK:
        input_entry.set_text(dialog->get_file()->get_path());
        char out_file_path[500];
        strcpy(out_file_path, dialog->get_file()->get_parent()->get_path().c_str());//(dialog->get_file()->get_parent()->get_path()) + "_out.png";
        strcat(out_file_path, "/out.png");
        output_entry.set_text(out_file_path);
        break;
    default:
        break;
    }

    delete dialog;
}

void MainWindow::hide_dialog(){
    err_dialog->hide();
}

struct rgba_pixel {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
};

constexpr unsigned int r_channel_idx = 0;
constexpr unsigned int g_channel_idx = 1;
constexpr unsigned int b_channel_idx = 2;
constexpr unsigned int a_channel_idx = 3;

std::vector<rgba_pixel> convert_cimg_to_rgba_buffer(const cimg_library::CImg<unsigned char>& img) {
    const unsigned int img_height = static_cast<unsigned int>(img.height());
    const unsigned int img_width = static_cast<unsigned int>(img.width());
    const unsigned int number_of_channels = static_cast<unsigned int>(img.spectrum());

    const bool has_r_channel = number_of_channels > r_channel_idx;
    const bool has_g_channel = number_of_channels > g_channel_idx;
    const bool has_b_channel = number_of_channels > b_channel_idx;
    const bool has_a_channel = number_of_channels > a_channel_idx;

    std::vector<rgba_pixel> rgba_buf(static_cast<std::size_t>(img_width) * img_height);
    for (unsigned int y = 0; y < img_height; ++y) {
        for (unsigned int x = 0; x < img_width; ++x) {
            const std::size_t pixel_idx = static_cast<std::size_t>(img_width) * y + x;
            rgba_buf[pixel_idx].r = has_r_channel ? *img.data(x, y, 0, r_channel_idx) : 0;
            rgba_buf[pixel_idx].g = has_g_channel ? *img.data(x, y, 0, g_channel_idx) : 0;
            rgba_buf[pixel_idx].b = has_b_channel ? *img.data(x, y, 0, b_channel_idx) : 0;
            rgba_buf[pixel_idx].a = has_a_channel ? *img.data(x, y, 0, a_channel_idx) : UCHAR_MAX;
        }
    }
    return rgba_buf;
}

void copy_rgba_buffer_to_cimg(const std::vector<rgba_pixel>& rgba_buf, cimg_library::CImg<unsigned char>& img) {
    const unsigned int img_height = static_cast<unsigned int>(img.height());
    const unsigned int img_width = static_cast<unsigned int>(img.width());
    const unsigned int number_of_channels = static_cast<unsigned int>(img.spectrum());

    const bool has_r_channel = number_of_channels > r_channel_idx;
    const bool has_g_channel = number_of_channels > g_channel_idx;
    const bool has_b_channel = number_of_channels > b_channel_idx;
    const bool has_a_channel = number_of_channels > a_channel_idx;

    for (unsigned int y = 0; y < img_height; ++y) {
        for (unsigned int x = 0; x < img_width; ++x) {
            const std::size_t pixel_idx = static_cast<std::size_t>(img_width) * y + x;
            if (has_r_channel) *img.data(x, y, 0, r_channel_idx) = rgba_buf[pixel_idx].r;
            if (has_g_channel) *img.data(x, y, 0, g_channel_idx) = rgba_buf[pixel_idx].g;
            if (has_b_channel) *img.data(x, y, 0, b_channel_idx) = rgba_buf[pixel_idx].b;
            if (has_a_channel) *img.data(x, y, 0, a_channel_idx) = rgba_buf[pixel_idx].a;
        }
    }
}

void edge_detect(const char *img_file_name, const char *img_out_path){
    const char *kernel_source = "\n"\
"const sampler_t sampler = CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;\n"\
"\n"\
"__kernel void edge_detection(\n"\
    "read_only image2d_t I,\n"\
    "write_only image2d_t O\n"\
")\n"\
"{\n"\
    "int gid_x = (int) get_global_id(0);\n"\
    "int gid_y = (int) get_global_id(1);\n"\
"\n"\
    "if (gid_x >= get_image_width(I) || gid_y >= get_image_height(I))\n"\
        "return;\n"\
"\n"\
    "uint4 p00 = read_imageui(I, sampler, (int2)(gid_x-1, gid_y-1));\n"\
    "uint4 p10 = read_imageui(I, sampler, (int2)(gid_x, gid_y-1));\n"\
    "uint4 p20 = read_imageui(I, sampler, (int2)(gid_x+1, gid_y-1));\n"\
"\n"\
    "uint4 p01 = read_imageui(I, sampler, (int2)(gid_x-1, gid_y));\n"\
    "uint4 p11 = read_imageui(I, sampler, (int2)(gid_x, gid_y));\n"\
    "uint4 p21 = read_imageui(I, sampler, (int2)(gid_x+1, gid_y));\n"\
"\n"\
    "uint4 p02 = read_imageui(I, sampler, (int2)(gid_x-1, gid_y+1));\n"\
    "uint4 p12 = read_imageui(I, sampler, (int2)(gid_x, gid_y+1));\n"\
    "uint4 p22 = read_imageui(I, sampler, (int2)(gid_x+1, gid_y+1));\n"\
"\n"\
    "uint p10gray = 0.33 * (p10.x + p10.y + p10.z);\n"\
"\n"\
    "uint p01gray = 0.33 * (p01.x + p01.y + p01.z);\n"\
    "uint p11gray = 0.33 * (p11.x + p11.y + p11.z);\n"\
    "uint p21gray = 0.33 * (p21.x + p21.y + p21.z);\n"\
"\n"\
    "uint p12gray = 0.33 * (p12.x + p12.y + p12.z);\n"\
"\n"\
    "uint s = -1 * (p10gray + p01gray + p21gray + p12gray) + p11gray * 4;\n"\
    "uint v = s > 200 ? 0 : s;\n"\
"\n"\
    "write_imageui(O, (int2)(gid_x, gid_y), (uint4)(v, v, v, 255));\n"\
"}\n";

    cl_context context;
    cl_platform_id platform;
    cl_uint num_platforms;
    cl_device_id device;
    cl_command_queue command_queue;
    cl_program program;
    cl_kernel kernel;
    cl_int errNum;

    errNum = clGetPlatformIDs(1, &platform, &num_platforms);
    if(errNum != CL_SUCCESS){
       cerr << "Errore nel prendere la piattaforma OpenCL";
    }

    errNum = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
    if(errNum != CL_SUCCESS){
       cerr << "Errore nel prendere il device OpenCL";
    }

    char* value;
    size_t valueSize;

    clGetDeviceInfo(device, CL_DEVICE_NAME, 0, NULL, &valueSize);
    value = (char*) malloc(valueSize);
    clGetDeviceInfo(device, CL_DEVICE_NAME, valueSize, value, NULL);
    cout << "Device: " << value << endl;
    free(value);

    cl_context_properties properties [] = {
       CL_CONTEXT_PLATFORM, (cl_context_properties) platform, 0
    };

    context = clCreateContext(properties, 1, &device, NULL, NULL, &errNum);
    if(errNum != CL_SUCCESS){
        cerr << "Errore nel context OpenCL";
    }

    const cl_queue_properties props[] = {
        CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE, 0
    };

    command_queue = clCreateCommandQueueWithProperties(context, device, props, &errNum);
    if(errNum != CL_SUCCESS){
        cout << "Errore nella creazione command queue OpenCL!" << endl;
    }

    program = clCreateProgramWithSource(context, 1, (const char **) &kernel_source, NULL, &errNum);
    if(errNum != CL_SUCCESS){
        cout << "Errore nella creazione del programma OpenCL!" << endl;
    }

    errNum = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if(errNum != CL_SUCCESS){
        cout << "Errore nella build dell'eseguibile OpenCL: " << errNum << endl;
        size_t log_size;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

        char *log = (char *)calloc(log_size, log_size);
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
        cout << log << endl;
    }

    kernel = clCreateKernel(program, "edge_detection", &errNum);
    if(errNum != CL_SUCCESS){
        cout << "Errore nel kernel OpenCL: " << errNum << endl;
    }

    // INPUT IMAGE //

    CImg<unsigned char> img_in(img_file_name);

    cl_image_format format = {
        CL_RGBA,
        CL_UNSIGNED_INT8,
    };

    cl_image_desc desc = {
        .image_type = CL_MEM_OBJECT_IMAGE2D,
        .image_width = (size_t) img_in.width(),
        .image_height = (size_t) img_in.height(),
        .image_row_pitch = 0,
        .image_slice_pitch = 0,
        .num_mip_levels = 0,
        .num_samples = 0,
        .buffer = NULL,
    };
    
    cl_mem input_img = clCreateImage(
        context,
        CL_MEM_READ_ONLY,
        (const cl_image_format *) &format,
        (const cl_image_desc *) &desc,
        NULL,
        &errNum
    );

    if(errNum != CL_SUCCESS){
        cerr << "Errore nella creazione dell'immagine di input!: " << errNum << endl;
    }

    // OUTPUT IMAGE //

    CImg<unsigned char> img_out(img_in.width(), img_in.height(), img_in.depth(), img_in.spectrum());
    //cout << img_out.data() << endl;

    format = {
        CL_RGBA,
        CL_UNSIGNED_INT8,
    };

    desc = {
        .image_type = CL_MEM_OBJECT_IMAGE2D,
        .image_width = (size_t) img_out.width(),
        .image_height = (size_t) img_out.height(),
        .image_row_pitch = 0,
        .image_slice_pitch = 0,
        .num_mip_levels = 0,
        .num_samples = 0,
        .buffer = NULL,
    };

    cl_mem output_img = clCreateImage(
        context,
        CL_MEM_WRITE_ONLY,
        (const cl_image_format *) &format,
        (const cl_image_desc *) &desc,
        NULL,
        &errNum
    );

    if(errNum != CL_SUCCESS){
        cerr << "Errore nella creazione dell'immagine di output!: " << errNum << endl;
    }

    size_t origins[3] = {0, 0, 0};
    size_t region[3] = {(size_t) img_in.width(), (size_t) img_in.height(), (size_t) 1};

    errNum = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input_img);
    errNum = clSetKernelArg(kernel, 1, sizeof(cl_mem), &output_img);

    auto rgba_buf = convert_cimg_to_rgba_buffer(img_in);
    errNum = clEnqueueWriteImage(command_queue, input_img, CL_FALSE, origins, region, 0, 0, rgba_buf.data(), 0, NULL, NULL);

    size_t global[2] = {(size_t) img_in.width(), (size_t) img_in.height()};
    clEnqueueNDRangeKernel(command_queue, kernel, 2, NULL, global, NULL, 0, NULL, NULL);

    errNum = clEnqueueReadImage(command_queue, output_img, CL_TRUE, origins, region, 0, 0, rgba_buf.data(), 0, NULL, NULL);

    copy_rgba_buffer_to_cimg(rgba_buf, img_out);
    img_out.save(img_out_path);
    img_out.display();

    clReleaseCommandQueue(command_queue);
    clReleaseContext(context);
    clReleaseDevice(device);
    clReleaseKernel(kernel);
    clReleaseProgram(program);

    clReleaseMemObject(input_img);
    clReleaseMemObject(output_img);

}

void MainWindow::on_button_clicked(){
    Glib::ustring img_in_path = input_entry.get_text();
    Glib::ustring img_out_path = output_entry.get_text();
    cout << img_in_path << endl;
    CImg<unsigned char> input_img;
    try{
        input_img.load(img_in_path.c_str());
    }
    catch (exception &err){
        err_dialog.reset(new Gtk::MessageDialog(*this,
            "\n\nFile non trovato\n\n",
            false,
            Gtk::MessageType::ERROR,
            Gtk::ButtonsType::NONE,
            true
        ));

        err_dialog->set_hide_on_close(true);
        
        err_dialog->show();

        return;

    }
    edge_detect(img_in_path.c_str(), img_out_path.c_str());
}

// ImgCanvas::ImgCanvas(Glib::RefPtr<Gdk::Pixbuf> img){
//     this->img = img;
// }

// void ImgCanvas::on_draw(const Cairo::RefPtr<Cairo::Context> &cr, int width, int height){
//     if(!img){
//         return;
//     }

//     Gdk::Cairo::set_source_pixbuf(cr, img, 
//         (width - img->get_width()) / 2,
//         (height - img->get_width()) / 2
//     );

//     cr->paint();
// }