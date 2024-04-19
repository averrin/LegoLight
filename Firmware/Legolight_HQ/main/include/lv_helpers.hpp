#pragma once
#include <lvgl.h>

lv_obj_t * textWidget(lv_obj_t *parent) {
    lv_obj_t * container;
    container = lv_obj_create(parent);
    lv_obj_set_size(container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    auto label1 = lv_label_create(container);
    lv_label_set_recolor(label1, true);                      /*Enable re-coloring by commands in the text*/
    return container;
}

lv_obj_t *insertButton(lv_obj_t *parent, std::string title, int c, int r)
{

    lv_obj_t *button;
    button = lv_btn_create(parent);
    lv_obj_set_size(button, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    auto label1 = lv_label_create(button);
    lv_label_set_text_fmt(label1, title.c_str());
    lv_obj_center(label1);

    lv_obj_set_grid_cell(button,
                         LV_GRID_ALIGN_STRETCH, c, 1,
                         LV_GRID_ALIGN_STRETCH, r, 1);
    return button;
}

void setRotation()
{
    amoled.setRotation(1);
    // rotation %= 4;
    lv_disp_drv_t *drv = lv_disp_get_default()->driver;
    drv->hor_res = amoled.width();
    drv->ver_res = amoled.height();
    lv_disp_drv_update(lv_disp_get_default(), drv);
}