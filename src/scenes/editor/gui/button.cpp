#include "button.hpp"
#include "lib/opengl.hpp"
#include "transform.hpp"
#include "../editor.hpp"

#include "lib/glm.hpp"

#include "render/func.hpp"

int GuiButton::getWidth(){
    return width;
}
int GuiButton::getHeight(){
    return height;
}
int GuiButton::getPosX(){
    return posX;
}
int GuiButton::getPosY(){
    return posY;
}

GuiButton & GuiButton::init(sceneEditor * scene){
    this->scene = scene;
    return *this;
}
GuiButton & GuiButton::init(sceneEditor * scene, int posX, int posY, int width, int height){
    this->scene=scene;
    setPosition(posX,posY);
    setSize(width,height);
    return *this;
}
GuiButton & GuiButton::setPosition(int posX, int posY){
    this->posX=posX;
    this->posY=posY;
    return *this;
}
GuiButton & GuiButton::setSize(int width, int height){
    this->width=width;
    this->height=height;
    return *this;
}

GuiButton & GuiButton::setBackgroundVisible(bool n){
    background = n;
    return *this;
}

bool GuiButton::pushEvent(SDL_Event * evt){
    bool used=false;
    switch(evt->type){
        case SDL_MOUSEBUTTONDOWN:{
            if(evt->button.button==SDL_BUTTON_LEFT){
                if(evt->button.x*scene->a.getAreaMultipler()>=posX&&evt->button.y*scene->a.getAreaMultipler()>=posY&&evt->button.x*scene->a.getAreaMultipler()<posX+width&&evt->button.y*scene->a.getAreaMultipler()<posY+height){
                    used=true;
                    pressed=true;
                }
            }
            break;
        }
        case SDL_MOUSEBUTTONUP:{
            if(evt->button.button==SDL_BUTTON_LEFT){
                if(pressed){
                    if(evt->button.x*scene->a.getAreaMultipler()>=posX&&evt->button.y*scene->a.getAreaMultipler()>=posY&&evt->button.x*scene->a.getAreaMultipler()<posX+width&&evt->button.y*scene->a.getAreaMultipler()<posY+height){
                        used=true;
                        clicked=true;
                    }
                    pressed=false;
                }
            }

            break;
        }
        case SDL_MOUSEMOTION:{
            if(pressed){
                used=true;
            }
            break;
        }
    }
    return used;
}

bool GuiButton::isClicked(){
    if(clicked){
        clicked=false;
        return true;
    }
    else{
        return false;
    }
}

bool GuiButton::isPressed(){
    return pressed;
}

GuiButton::GuiButton(){
	model = new glm::mat4;
}

GuiButton::~GuiButton(){
	delete (glm::mat4*)model;
}

GuiButton & GuiButton::render(){

    xReset((glm::mat4*)model);
    if(!pressed){
        xTranslate((glm::mat4*)model, posX,posY);
    }
    else{
        xTranslate((glm::mat4*)model, posX+2,posY+2);
    }
    xScale((glm::mat4*)model,width, height);
    scene->shGui.select();
    scene->shGui.setM((glm::mat4*)model);

    scene->a.square_vert->bind().attrib(0,2,GL_FLOAT);
	scene->a.square_uv->bind().attrib(1,2,GL_FLOAT);

    if(background){
        scene->thMan["button"].select();
        scene->a.square_vert->draw(GL_TRIANGLES);
    }
    if(texture!=NULL){
        texture->select();
        scene->a.square_vert->draw(GL_TRIANGLES);
    }
    return *this;
}

GuiButton & GuiButton::setTexture(Texture * texture){
    this->texture = texture;
    return *this;
}
