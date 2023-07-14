#if defined(__linux__)
#include "../conio.h"
#else
#include <conio.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <libobsensor/h/ObTypes.h>
#include <libobsensor/h/Frame.h>
#include <libobsensor/h/Pipeline.h>
#include <libobsensor/h/StreamProfile.h>
#include <libobsensor/h/Error.h>
#include <libobsensor/h/Device.h>
#include <libobsensor/h/Context.h>
#include <libobsensor/h/Filter.h>

#define KEY_ESC 27
#define KEY_R 82
#define KEY_r 114

ob_error *   error    = NULL;  // 用于SDK 接口错误信息返回
ob_pipeline *pipeline = NULL;  // pipeline, 用于连接设备后打开Color和Depth流

void check_error(ob_error *error) {
    if(error) {
        printf("ob_error was raised: \n\tcall: %s(%s)\n", ob_error_function(error), ob_error_args(error));
        printf("\tmessage: %s\n", ob_error_message(error));
        printf("\terror type: %d\n", ob_error_exception_type(error));
        exit(EXIT_FAILURE);
    }
}

// 保存点云数据到ply
void save_points_to_ply(ob_frame *frame, const char *fileName) {
    int pointsSize = ob_frame_data_size(frame, &error) / sizeof(ob_point);
    check_error(error);

    FILE *fp = fopen(fileName, "wb+");
    fprintf(fp, "ply\n");
    fprintf(fp, "format ascii 1.0\n");
    fprintf(fp, "element vertex %d\n", pointsSize);
    fprintf(fp, "property float x\n");
    fprintf(fp, "property float y\n");
    fprintf(fp, "property float z\n");
    fprintf(fp, "end_header\n");

    ob_point *point = (ob_point *)ob_frame_data(frame, &error);
    check_error(error);
    for(int i = 0; i < pointsSize; i++) {
        fprintf(fp, "%.3f %.3f %.3f\n", point->x, point->y, point->z);
        point++;
    }

    fflush(fp);
    fclose(fp);
}

// 保存彩色点云数据到ply
void save_rgb_points_to_ply(ob_frame *frame, const char *fileName) {
    int pointsSize = ob_frame_data_size(frame, &error) / sizeof(ob_color_point);
    check_error(error);

    FILE *fp = fopen(fileName, "wb+");
    fprintf(fp, "ply\n");
    fprintf(fp, "format ascii 1.0\n");
    fprintf(fp, "element vertex %d\n", pointsSize);
    fprintf(fp, "property float x\n");
    fprintf(fp, "property float y\n");
    fprintf(fp, "property float z\n");
    fprintf(fp, "property uchar red\n");
    fprintf(fp, "property uchar green\n");
    fprintf(fp, "property uchar blue\n");
    fprintf(fp, "end_header\n");

    ob_color_point *point = (ob_color_point *)ob_frame_data(frame, &error);
    check_error(error);

    for(int i = 0; i < pointsSize; i++) {
        fprintf(fp, "%.3f %.3f %.3f %d %d %d\n", point->x, point->y, point->z, (int)point->r, (int)point->g, (int)point->b);
        point++;
    }

    fflush(fp);
    fclose(fp);
}

int main(int argc, char **argv) {
    ob_set_logger_severity(OB_LOG_SEVERITY_ERROR, &error);
    check_error(error);

    // 创建PipeLine 用于连接设备后打开Color和Depth流
    pipeline = ob_create_pipeline(&error);
    check_error(error);

    // 创建config，用于配置 Color 和 Depth 流的 分辨率、帧率、格式
    ob_config *config = ob_create_config(&error);
    check_error(error);

    // 配置Depth流
    ob_stream_profile *     depth_profile = NULL;
    ob_stream_profile_list *profiles      = ob_pipeline_get_stream_profile_list(pipeline, OB_SENSOR_DEPTH, &error);
    check_error(error);
    // 打开Depth Sensor默认的profile,可通过配置文件配置
    if(profiles) {
        depth_profile = ob_stream_profile_list_get_profile(profiles, 0, &error);
        check_error(error);
    }
    ob_config_enable_stream(config, depth_profile, &error);  // 使能配置
    check_error(error);

    // 配置Color流
    ob_stream_profile *color_profile = NULL;
    profiles                         = ob_pipeline_get_stream_profile_list(pipeline, OB_SENSOR_COLOR, &error);
    if(error) {
        printf("Current device is not support color sensor!\n");
        // 如果不存在Color Sensor 点云转换分辨率配置为深度分辨率
        ob_config_set_d2c_target_resolution(config, ob_video_stream_profile_width(depth_profile, &error), ob_video_stream_profile_height(depth_profile, &error),
                                            &error);
        ob_config_set_depth_scale_require(config, false, &error);
        error = NULL;
    }

    // 打开Color Sensor默认的profile,可通过配置文件配置
    if(profiles) {
        color_profile = ob_stream_profile_list_get_profile(profiles, 0, &error);
    }

    // 使能配置
    if(color_profile) {
        ob_config_enable_stream(config, color_profile, &error);
        check_error(error);
    }

    // 获取device句柄
    ob_device *device = ob_pipeline_get_device(pipeline, &error);
    check_error(error);

    // 开启D2C对齐, 生成RGBD点云时需要开启
    ob_config_set_align_mode(config, ALIGN_D2C_HW_MODE, &error);
    check_error(error);

    // 通过config 启动pipeline
    ob_pipeline_start_with_config(pipeline, config, &error);
    check_error(error);

    // 创建点云Filter对象（点云Filter创建时会在Pipeline内部获取设备参数, 所以尽量在Filter创建前配置好设备）
    ob_filter *point_cloud = ob_create_pointcloud_filter(&error);
    check_error(error);

    // 从pipeline获取当前开流的相机参数，并传入到点云filter
    ob_camera_param camera_param = ob_pipeline_get_camera_param(pipeline, &error);
    check_error(error);
    ob_pointcloud_filter_set_camera_param(point_cloud, camera_param, &error);
    check_error(error);

    // 操作提示
    printf("Press R to create rgbd pointCloud and save to ply file!\n");
    printf("Press D to create depth pointCloud and save to ply file!\n");
    printf("Press ESC to exit!\n");

    int  count          = 0;
    bool points_created = false;
    while(true) {
        if(kbhit()) {
            int key = getch();
            // 按ESC键退出
            if(key == KEY_ESC) {
                break;
            }
            if(key == 'R' || key == 'r') {
                count          = 0;
                points_created = false;
                // 生成彩色点云并保存
                printf("Save RGBD pointCloud ply file...\n");
                // 限制最多重复10次
                while(count++ < 10) {
                    // 等待一帧数据，超时时间为100ms
                    ob_frame *frameset = ob_pipeline_wait_for_frameset(pipeline, 100, &error);
                    check_error(error);
                    if(frameset != NULL) {
                        ob_pointcloud_filter_set_point_format(point_cloud, OB_FORMAT_RGB_POINT, &error);
                        check_error(error);
                        ob_frame *pointsFrame = ob_filter_process(point_cloud, frameset, &error);
                        check_error(error);
                        if(pointsFrame != NULL) {
                            save_rgb_points_to_ply(pointsFrame, "rgb_points.ply");
                            printf("rgb_points.ply Saved\n");
                            ob_delete_frame(pointsFrame, &error);
                            check_error(error);
                            points_created = true;
                        }
                        ob_delete_frame(frameset, &error);  // 销毁frameSet 回收内存
                        check_error(error);
                        if(points_created) {
                            break;
                        }
                    }
                }
            }
            else if(key == 'D' || key == 'd') {
                count          = 0;
                points_created = false;
                // 生成点云并保存
                printf("Save depth pointCloud to ply file...\n");
                // 限制最多重复10次
                while(count++ < 10) {
                    // 等待一帧数据，超时时间为100ms
                    ob_frame *frameset = ob_pipeline_wait_for_frameset(pipeline, 100, &error);
                    check_error(error);
                    if(frameset != NULL) {
                        ob_pointcloud_filter_set_point_format(point_cloud, OB_FORMAT_POINT, &error);
                        check_error(error);
                        ob_frame *pointsFrame = ob_filter_process(point_cloud, frameset, &error);
                        check_error(error);
                        if(pointsFrame != NULL) {
                            save_points_to_ply(pointsFrame, "points.ply");
                            printf("points.ply Saved\n");
                            ob_delete_frame(pointsFrame, &error);
                            check_error(error);
                            points_created = true;
                        }
                        ob_delete_frame(frameset, &error);  // 销毁frameSet 回收内存
                        check_error(error);
                        if(points_created) {
                            break;
                        }
                    }
                }
            }
        }
        ob_frame *frameset = ob_pipeline_wait_for_frameset(pipeline, 100, &error);
        check_error(error);
        if(frameset) {
            ob_delete_frame(frameset, &error);  // 销毁frameSet 回收内存
            check_error(error);
        }
    }

    // 销毁点云Filter
    ob_delete_filter(point_cloud, &error);
    check_error(error);

    // 停止pipeline
    ob_pipeline_stop(pipeline, &error);
    check_error(error);

    // 销毁pipeline
    ob_delete_pipeline(pipeline, &error);
    check_error(error);

    // 销毁config
    ob_delete_config(config, &error);
    check_error(error);

    // 销毁profile
    ob_delete_stream_profile(depth_profile, &error);
    check_error(error);

    // 销毁profile
    ob_delete_stream_profile(color_profile, &error);
    check_error(error);

    // 销毁profile list
    ob_delete_stream_profile_list(profiles, &error);
    check_error(error);

    return 0;
}
