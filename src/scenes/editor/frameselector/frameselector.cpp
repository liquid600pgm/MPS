#include "frameselector.hpp"
#include "../editor.hpp"
#include "transform.hpp"
#include <sstream>
#include "timestep.hpp"

#include "render/func.hpp"

void frameSelector::init(sceneEditor * scene){
    this->scene = scene;

    text_currentframe.init(scene);

    glGenBuffers(1, &linebuffer);
    glBindBuffer(GL_ARRAY_BUFFER, linebuffer);
    float linedata[] = {0,0, 1,0, 1,0, 1,1, 1,1, 0,1, 0,1, 0,0};
    glBufferData(GL_ARRAY_BUFFER, sizeof(linedata), linedata, GL_STATIC_DRAW);

    but_newframe.init(scene).setTexture(&scene->thMan["fs/newframe"]).setSize(32, 32).setBackgroundVisible(false);
    but_removeframe.init(scene).setTexture(&scene->thMan["fs/removeframe"]).setSize(32, 32).setBackgroundVisible(false);
    but_newframetype.init(scene).setTexture(&scene->thMan["fs/opt_blank"]).setSize(24,32).setBackgroundVisible(false);
}

frameSelector::~frameSelector(){
    glDeleteBuffers(1, &linebuffer);
}


bool frameSelector::pushEvent(SDL_Event * evt){
    used=false;

    switch(evt->type){
        case SDL_MOUSEBUTTONDOWN:{
            if(evt->button.y*scene->a.getAreaMultipler()>scene->a.getAreaHeight()-height){
                used=true;
            }
            break;
        }
    }

    if(but_newframe.pushEvent(evt)){
        used=true;
    }
    if(but_newframe.isClicked()){
        if(newFrameType==0){
            scene->frameMan.createFrame(scene->frameMan.getCurrentFrame()->getWidth(),scene->frameMan.getCurrentFrame()->getHeight())->createLayer();
        }
        else{
            scene->frameMan.duplicateFrame();
        }
        scene->changeFrame(scene->frameMan.getCurrentFrameIndex()+1);
        updateCurrentFrameText();
    }

    if(but_removeframe.pushEvent(evt)){
        used=true;
    }
    if(but_removeframe.isClicked()){
        if(!scene->frameMan.removeFrame()){
            scene->actionlog.addMessage("Cannot remove frame");
        }
        scene->changeFrame(scene->frameMan.getCurrentFrameIndex());
        updateCurrentFrameText();
    }

    if(but_newframetype.pushEvent(evt)){
        used=true;
    }
    if(but_newframetype.isClicked()){
        newFrameType=!newFrameType;
        if(newFrameType==0){
            but_newframetype.setTexture(&scene->thMan["fs/opt_blank"]);
            scene->actionlog.addMessage("New frame type: BLANK");
        }
        else{
            but_newframetype.setTexture(&scene->thMan["fs/opt_duplicate"]);
            scene->actionlog.addMessage("New frame type: DUPLICATE");
        }
    }

    return used;
}

bool frameSelector::pushGlobalEvent(GlobalEvent evt){
    bool used=false;

    switch(evt){
        case GlobalEvent::GuiChange:{
            updateCurrentFrameText();
            used=true;
            break;
        }
        default:break;
    }

    return used;
}

void frameSelector::update(){
    index_difference_prev = index_difference;
    index_difference *=0.6;

    but_newframe.update();
    but_newframetype.update();
    but_removeframe.update();
}

void frameSelector::render(float alpha){
    areawidth=scene->a.getAreaWidth();
    areaheight=scene->a.getAreaHeight();

    int frameY = scene->a.getAreaHeight()-height+5;
    int frameHeight = height-10;

    if(prev_index!=scene->frameMan.getCurrentFrameIndex()){
        index_difference+=(scene->frameMan.getCurrentFrameIndex()-prev_index)*frameHeight;
        prev_index = scene->frameMan.getCurrentFrameIndex();
        updateCurrentFrameText();
    }


    Shader & sh = scene->shGui;
    Shader & shColor = scene->shGuiColor;

    {//background
        xReset(&model);
        xTranslate(&model, 0, areaheight-height-8);
        xScale(&model,areawidth, height+8);
        sh.select();
        scene->thMan["fs/background1"].select();
        sh.setM(&model);
        scene->a.square_vert->bind().attrib(0,2,GL_FLOAT);
		scene->a.square_uv->bind().attrib(1,2,GL_FLOAT);
		scene->a.square_vert->draw(GL_TRIANGLES);

        {//top
            int offsetX = areawidth/2-topWidth/2;
            int offsetY = areaheight-height-topHeight;

            Texture & background_top = scene->thMan["fs/background_top"];
            background_top.select();
            xReset(&model);
            xTranslate(&model, offsetX-8,offsetY-8);
            xScale(&model, background_top.getWidth(),background_top.getHeight());
            sh.setM(&model);
            scene->a.square_vert->draw(GL_TRIANGLES);

            but_newframe.setPosition(offsetX+topWidth-35,offsetY+topHeight-35);
            but_newframe.render(alpha);

            but_newframetype.setPosition(but_newframe.getPosX()-but_newframetype.getWidth(),but_newframe.getPosY());
            but_newframetype.render(alpha);

            but_removeframe.setPosition(offsetX+3,offsetY+topHeight-35);
            but_removeframe.render(alpha);

        }
    }
    {//frames

        scene->a.square_vert->bind().attrib(0,2,GL_FLOAT);
		scene->a.square_uv->bind().attrib(1,2,GL_FLOAT);

        int offset;
        int frameseek = areawidth/height/2+1;
        for(int i=scene->frameMan.getCurrentFrameIndex()-frameseek; i<=scene->frameMan.getCurrentFrameIndex()+frameseek;i++){
            if(i>=0&&i<scene->frameMan.getFrameCount()){
                offset = i-scene->frameMan.getCurrentFrameIndex();

                xReset(&model);
                xTranslate(&model, areawidth/2 - frameHeight/2 + offset*(height) + alphize(alpha, index_difference_prev, index_difference), frameY);
                float proportions = scene->frameMan.getFrame(i)->getProportions();
                if(proportions>1.0){
                    xScale(&model,frameHeight, frameHeight/proportions);
                }
                else{
                    xScale(&model,frameHeight*proportions, frameHeight);
                }
                scene->frameMan.getFrame(i)->getSelectedLayer()->bindTexture();

                shColor.select();
                shColor.setM(&model);
                glBindBuffer(GL_ARRAY_BUFFER, linebuffer);
                glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,(void*)0);

                if(i==scene->frameMan.getCurrentFrameIndex()){
                    shColor.setUniform("COLOR",1,0,0,1);
                    glLineWidth(3);
                }
                else{
                    shColor.setUniform("COLOR",0,0,0,0.5);
                    glLineWidth(1);
                }

                glDrawArrays(GL_LINES, 0, 8);

                sh.select();
                sh.setM(&model);

                scene->a.square_vert->bind().attrib(0,2,GL_FLOAT);
				scene->a.square_uv->bind().attrib(1,2,GL_FLOAT);
				scene->a.square_vert->draw(GL_TRIANGLES);
            }
        }
    }
    {//frame info
        text_currentframe.setPosition(areawidth/2,areaheight-height-28);
        text_currentframe.render();
    }
}

void frameSelector::updateCurrentFrameText(){
    std::stringstream ss;
    ss<<scene->frameMan.getCurrentFrameIndex()+1<<"/"<<scene->frameMan.getFrameCount();
    text_currentframe.changeText(ss.str(),scene->a.font24, 255, 255, 255);

    text_currentframe.setAlign(1, 0);
}

int frameSelector::getHeight(){
    return height;
}
