// module name here
extern "C" const char * getName(void) { return "demo";}

extern "C" void load(void);
extern "C" void unload(void);
extern "C" char * serve(const char * param);

// what you want to do when being loaded
void load() {
	return;
}

// what you want to do when being unloaded
void unload() {
	return;
}

char body[] = "\
<!DOCTYPE html><html><head><title>Life Game</title></head><body><center><p><a href=\"https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life\">Conway\'s Game of Life</a></p><canvas id=\"canvas\" onclick=start()>Your browser doen't support HTML5 canvas, please use a newer browser.</canvas></center><script>function newEmptyWorld(){for(var e=new Array,t=0;width>t;t++){e[t]=new Array;for(var o=0;height>o;o++)e[t][o]=!1}return e}function genLives(e){function t(o,a,r){if(-1==o&&(o=width-1),o==width&&(o=0),-1==a&&(a=height-1),a==height&&(a=0),0>o||o>=width||0>a||a>=height||e[o][a])return 0;e[o][a]=!0;var l=1;return Math.random()<r&&(l+=t(o+1,a+1,r*expDecay)),Math.random()<r&&(l+=t(o+1,a,r*expDecay)),Math.random()<r&&(l+=t(o+1,a-1,r*expDecay)),Math.random()<r&&(l+=t(o,a+1,r*expDecay)),Math.random()<r&&(l+=t(o,a-1,r*expDecay)),Math.random()<r&&(l+=t(o-1,a+1,r*expDecay)),Math.random()<r&&(l+=t(o-1,a,r*expDecay)),Math.random()<r&&(l+=t(o-1,a-1,r*expDecay)),l}for(var o=0;width>o;o++)for(var a=0;height>a;a++)Math.random()<genProb&&(liveCount+=t(o,a,expProb))}function evolveWorld(e){function t(t,o){return-1==t&&(t=width-1),t==width&&(t=0),-1==o&&(o=height-1),o==height&&(o=0),0>t||t>=width||0>o||o>=height?!1:e[t][o]}evolveFlag=!0;for(var o=newEmptyWorld(),a=0;width>a;a++)for(var r=0;height>r;r++){var l=0;switch(t(a+1,r+1)&&l++,t(a+1,r)&&l++,t(a+1,r-1)&&l++,t(a,r+1)&&l++,t(a,r-1)&&l++,t(a-1,r+1)&&l++,t(a-1,r)&&l++,t(a-1,r-1)&&l++,l){case 2:o[a][r]=e[a][r];break;case 3:o[a][r]=!0}}return evolveFlag=!1,o}function drawWorld(){for(var e=0;width>e;e++)for(var t=0;height>t;t++)oldWorld[e][t]!=world[e][t]&&(world[e][t]?ctx.fillStyle=aliveColor:ctx.fillStyle=deadColor,ctx.fillRect(e*blocksize,t*blocksize,blocksize,blocksize))}function update(){oldWorld=world,world=evolveWorld(world),drawWorld()}function autoevolve(){setTimeout(autoevolve,100),evolveFlag||update()}var width=256,height=128,blocksize=4,genProb=5e-4,expProb=.3,expDecay=.9,canvas=document.getElementById(\"canvas\"),ctx=canvas.getContext(\"2d\");canvas.width=width*blocksize,canvas.height=height*blocksize,ctx.fillStyle=\"#000000\",ctx.fillRect(0,0,canvas.width,canvas.height);var liveCount=0,evolveFlag=!1,aliveColor=\"#00c300\",deadColor=\"#0a0a0a\";start=function(){autoevolve(),start=function(){alert(\"Already started!\")}},world=newEmptyWorld(),genLives(world),world=evolveWorld(world);var oldWorld=newEmptyWorld();drawWorld();</script></body></html>\
\n";

// serve function which takes user's parameter after ?
char * serve(const char * param) {
	return body;
}
