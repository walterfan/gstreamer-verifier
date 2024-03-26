#pragma once

#include <gst/gst.h>
#include <glib.h>
#include <memory>
#include <thread>
#include <vector>
#include <map>
#include <atomic>
#include "string_util.h"
#include "file_util.h"
#include "pipeline_config.h"

namespace hefei {

constexpr auto CONFIG_FOLDER = "./etc";
constexpr auto CONFIG_FILE = "config.yaml";

enum class BuildProgress {
    NOT_STARTED = 0,
    MADE_ELEMENTS,
    ADDED_ELEMENTS,
    LINKED_ELEMENTS,
    UNLINKED_ELEMENTS,
    REMOVED_ELEMENTS,
    RELEASED_ELEMENTS
};

class PipelineBuilder {
public:
    PipelineBuilder(AppConfig& appConfig);
    virtual ~PipelineBuilder();

    int init(const std::string& pipeline_name);
    int clean();
    int build();
    int start();
    int stop();

    AppConfig& get_app_config() { return m_app_config; }
    int add_probe(const ProbeConfigItem& probe_config_item);

private:
    int init_gst(int argc, char * argv[]);
    GstElement* create_element(const std::string& factory, const std::string& name);
    GstElement* get_element(const std::string& name);
    GstPad* get_static_pad(const std::string& ele_name, const std::string& pad_name);

    bool add_element(const std::string& name);
    bool del_element(const std::string& name);

    bool link_elements();
    bool unlink_elements();

    void update_build_progress(BuildProgress progress);

    static gboolean on_bus_msg(GstBus* bus, GstMessage* msg, gpointer data);
    static void on_src_pad_added(GstElement* element, GstPad* pad, gpointer data);
    
    static GstPadProbeReturn probe_pad_callback(GstPad *pad, GstPadProbeInfo *info, gpointer user_data);

    void on_bus_msg_eos();
    void on_bus_msg_error(GstMessage* msg);
    void on_bus_msg_warning(GstMessage* msg);
    void on_state_changed(GstMessage* msg);
    void on_stream_started(GstMessage* msg);
    void on_stream_status(GstMessage* msg);
    void on_bus_msg_qos(GstMessage* msg);
    void on_bus_msg_buffering_stats(GstMessage* msg);

    std::map<std::string, GstElement*> m_elements;

    GstBus* m_bus;
    GMainLoop* m_loop;
    GstElement* m_pipeline;
    
    std::string m_pipeline_name;
    std::shared_ptr<PipelineConfig> m_pipeline_config;

    std::vector<std::pair<GstElement*, GstElement*>> m_linked_elements;
    std::vector<std::pair<GstPad*, GstPad*>> m_linked_pads;

    std::atomic<unsigned long> m_probe_count;
    BuildProgress m_build_progress;
    AppConfig m_app_config;
};

} //namespace hefei