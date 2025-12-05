#include <iostream>
#include <SDL2/SDL.h>
#include <unistd.h>
#include <chrono>
#include <cstdlib>

using namespace std;

void map_to_surface(char* map,SDL_Surface* surface,bool* active_map,const int chunk_size,int width, int height);
void simulate_chunk(char* map,bool* active_map_buffer,int x,int y,const int chunk_size,const int map_width,const int map_height);
void sim_tick(char* map,bool* active_map,bool* active_map_buffer,const int chunk_size,const int map_width,const int map_height);

int main(int argc, char **argv){
	if(argc < 3){
		cout << "NOT ENOUGH ARGS" << endl;
		return -1;
	}
	const int default_chunk_size = 1;
	int width,height,chunk_size;
	if(argc==3){
		try{
			width = stoi(argv[1]);
			height = stoi(argv[2]);
		}
		catch(invalid_argument& e){
			cout << "INVALID WIDTH OR HEIGHT" << endl;
			return -1;
		}
		catch(out_of_range& e){
			cout << "WIDHT OR HEIGHT OUT OF RANGE" << endl;
			return -1;
		}
		chunk_size = default_chunk_size;
		cout << "DEFAULT CHUNK SIZE ASSUMED: " << default_chunk_size << endl;
	}
	else if(argc==4){
		try{
			width = stoi(argv[1]);
			height = stoi(argv[2]);
			chunk_size = stoi(argv[3]);
		}
		catch(invalid_argument& e){
			cout << "INVALID WIDTH, HEIGHT OR DIVISOR" << endl;
			return -1;
		}
		catch(out_of_range& e){
			cout << "WIDHT,HEIGHT OR CHUNK SIZE OUT OF RANGE" << endl;
			return -1;
		}
	}
	srand(time(0));
	char* map = (char*)malloc(width*height);
	bool* active_map= (bool*)malloc((width/chunk_size)*(height/chunk_size));
	bool* active_map_buffer= (bool*)malloc((width/chunk_size)*(height/chunk_size));
	if(SDL_Init(SDL_INIT_EVERYTHING) != 0){
		cout << "SDL INIT ERR: " << SDL_GetError();
	}
	
	SDL_Window* window = SDL_CreateWindow("sand_sim",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,width,height,0);
	bool close = false;
	
	SDL_Renderer* renderer = SDL_CreateRenderer(window,-1,0);
	
	SDL_Surface* screen_surface = SDL_CreateRGBSurface(0,width,height,32,0,0,0,0);
	
	SDL_Rect rect;
	rect.x=0;
	rect.y=0;
	rect.w=width;
	rect.h=height;
	
	memset(map,0,width*height);
	memset(active_map,0,(width/chunk_size)*(height/chunk_size));
	memset(active_map_buffer,0,(width/chunk_size)*(height/chunk_size));
	*(map+width*442+22)=1;;
	unsigned int fps=60;
	double sleep_time=0;


	chrono::time_point<chrono::high_resolution_clock,chrono::nanoseconds> fps_clock;
	chrono::duration<double> diff;
	while(!close){
		fps_clock = chrono::high_resolution_clock::now();
		SDL_Event event;
		while(SDL_PollEvent(&event)){
			switch(event.type){
				case SDL_QUIT:
					close = true;
					break;
			}
		}
		int x,y;
		if(SDL_GetMouseState(&x,&y) & SDL_BUTTON_LMASK){
			if(x<width-1 && y<height-1 && x>1 && y>1){
				char* point = map+y*width+x;
				*(point) = 1;
				*(point+1) = 1;
				*(point-1) = 1;
				*(point-width) = 1;
				*(point-width+1) = 1;
				*(point-width-1) = 1;
				*(point+width) = 1;
				*(point+width+1) = 1;
				*(point+width-1) = 1;
				*(active_map_buffer+((int)(y/chunk_size)*width+x)/chunk_size)=1;
			}
		}
		else if(SDL_GetMouseState(&x,&y) & SDL_BUTTON_RMASK){
			if(x<width-1 && y<height-1 && x>1 && y >1){
				char* point = map+y*width+x;
				*(point) = 2;
				*(point+1) = 2;
				*(point-1) = 2;
				*(point-width) = 2;
				*(point-width+1) = 2;
				*(point-width-1) = 2;
				*(point+width) = 2;
				*(point+width+1) = 2;
				*(point+width-1) = 2;
			}
		}
		sim_tick(map,active_map,active_map_buffer,chunk_size,width,height);
		SDL_LockSurface(screen_surface);
		map_to_surface(map,screen_surface,active_map,chunk_size,width,height);
		SDL_UnlockSurface(screen_surface);
		
		SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer,screen_surface);
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer,tex,NULL,&rect);
		SDL_RenderPresent(renderer);
		SDL_DestroyTexture(tex);
		
		
		diff = chrono::high_resolution_clock::now()-fps_clock;
		fps = 1/diff.count();
		sleep_time = max((double)0,(double)1/60 - (double)1/fps);
		//Sleep(sleep_time*1000);
		diff = chrono::high_resolution_clock::now()-fps_clock;
		fps = 1/diff.count();
	}
	SDL_DestroyRenderer(renderer);
	SDL_FreeSurface(screen_surface);
	free(map);
	free(active_map);
	cout << "SIM_EXIT" << endl;
	return 0;
}


void map_to_surface(char* map,SDL_Surface* surface,bool* active_map,const int chunk_size,int width,int height){
	Uint32 bg = 0;
	Uint32 bg_green = 683560;
	Uint32 sand = 15453776;
	Uint32 barrier = 29368894;
	for(int i=0;i<height;i++){
		for(int j=0;j<width;j++){
			Uint32* pixel = (Uint32*)((Uint8*)surface->pixels + i * surface->pitch + j * surface->format->BytesPerPixel);
			switch(*(map + i*width+j)){
				case 0:
					if(*(active_map+i/chunk_size*width/chunk_size+j/chunk_size)){
						*pixel = bg_green;
						break;
					}
					*pixel = bg;
					break;
				case 1:
					*pixel = sand;
					break;
				case 2:
					*pixel = barrier;
					break;
			}

		}
	}
}
void inline reset_active_map_buffer(bool* active_map_buffer,const int chunk_size,const int map_width,const int map_height){
	memset(active_map_buffer,0,(map_width/chunk_size)*(map_height/chunk_size));
}
void inline copy_active_map_from_buffer(bool* active_map,bool* active_map_buffer,const int chunk_size,const int map_height,const int map_width){
	memcpy(active_map,active_map_buffer,(map_width/chunk_size)*(map_height/chunk_size));
}
void sim_tick(char* map,bool* active_map,bool* active_map_buffer,const int chunk_size,const int map_width,const int map_height){
	copy_active_map_from_buffer(active_map,active_map_buffer,chunk_size,map_height,map_width);
	reset_active_map_buffer(active_map_buffer,chunk_size,map_width,map_height);
	for(unsigned int i=0;i<map_height/chunk_size;i++){
		for(unsigned int j=0;j<map_width/chunk_size;j++){
			if(*(active_map+i*map_width/chunk_size+j)){
				simulate_chunk(map,active_map_buffer,j,i,chunk_size,map_width,map_height);
			}
		}
	}
}

void simulate_chunk(char* map,bool* active_map_buffer,int x,int y,const int chunk_size,const int map_width,const int map_height){
	int xlimit = min((x+1)*chunk_size,map_width-1),ylimit = min((y+1)*chunk_size,map_height-1);
	for(int i=y*chunk_size;i<=ylimit && x != map_height/chunk_size;i++){
		for(int j=x*chunk_size;j<xlimit;j++){
			char* point = map+i*map_width+j;
			if(*point == 1){
				char* point_below = point+map_width*2;
				if(*(point+map_width) == 0){
					*point = 0;
					*(point+map_width) = 1;
					*(active_map_buffer+i/chunk_size*map_width/chunk_size+map_width/chunk_size +j/chunk_size)=1;
					if(i%chunk_size == 0){
						*(active_map_buffer+(i-1)/chunk_size*map_width/chunk_size+j/chunk_size) = 1;
					}
					if(j%chunk_size == 0){
						*(active_map_buffer+(i-1)/chunk_size*map_width/chunk_size+(j-1)/chunk_size) = 1;
						*(active_map_buffer+(i-1)/chunk_size*map_width/chunk_size+(j+1)/chunk_size) = 1;
						*(active_map_buffer+(i)/chunk_size*map_width/chunk_size+(j-1)/chunk_size) = 1;
						*(active_map_buffer+(i)/chunk_size*map_width/chunk_size+(j+1)/chunk_size) = 1;
					}
				}
				else if( *(point_below+1)== 0 && *(point+1)==0){
					*point = 0;
					*(point+map_width*2+1) = 1;
					*(active_map_buffer+i/chunk_size*map_width/chunk_size+(j+1)/chunk_size)=1;
					if(i%chunk_size == 0){
						*(active_map_buffer+(i-1)/chunk_size*map_width/chunk_size+j/chunk_size) = 1;
					}
					if(j%chunk_size == 0){
						*(active_map_buffer+(i-1)/chunk_size*map_width/chunk_size+(j-1)/chunk_size) = 1;
						*(active_map_buffer+(i-1)/chunk_size*map_width/chunk_size+(j+1)/chunk_size) = 1;
						*(active_map_buffer+(i)/chunk_size*map_width/chunk_size+(j-1)/chunk_size) = 1;
						*(active_map_buffer+(i)/chunk_size*map_width/chunk_size+(j+1)/chunk_size) = 1;
					}
					
				}
				else if(*(point_below-1) == 0&& *(point-1)==0){
					*point = 0;
					*(point+map_width*2-1) = 1;
					*(active_map_buffer+i/chunk_size*map_width/chunk_size+(j-1)/chunk_size)=1;
					if(i%chunk_size == 0){
						*(active_map_buffer+(i-1)/chunk_size*map_width/chunk_size+j/chunk_size) = 1;
					}
					if(j%chunk_size == 0){
						*(active_map_buffer+(i-1)/chunk_size*map_width/chunk_size+(j-1)/chunk_size) = 1;
						*(active_map_buffer+(i-1)/chunk_size*map_width/chunk_size+(j+1)/chunk_size) = 1;
						*(active_map_buffer+(i)/chunk_size*map_width/chunk_size+(j-1)/chunk_size) = 1;
						*(active_map_buffer+(i)/chunk_size*map_width/chunk_size+(j+1)/chunk_size) = 1;
					}
				}
			}
		}
		for(int j=xlimit-1;j>=xlimit-chunk_size;j--){
			char* point = map+i*map_width+j;
			if(*point == 1){
				char* point_below = point+map_width*2;
				if(*(point+map_width) == 0){
					*point = 0;
					*(point+map_width) = 1;
					*(active_map_buffer+i/chunk_size*map_width/chunk_size+map_width/chunk_size +j/chunk_size)=1;
					if(i%chunk_size == 0){
						*(active_map_buffer+(i-1)/chunk_size*map_width/chunk_size+j/chunk_size) = 1;
					}
					if(j%chunk_size == 0){
						*(active_map_buffer+(i-1)/chunk_size*map_width/chunk_size+(j-1)/chunk_size) = 1;
						*(active_map_buffer+(i-1)/chunk_size*map_width/chunk_size+(j+1)/chunk_size) = 1;
						*(active_map_buffer+(i)/chunk_size*map_width/chunk_size+(j-1)/chunk_size) = 1;
						*(active_map_buffer+(i)/chunk_size*map_width/chunk_size+(j+1)/chunk_size) = 1;
					}
				}
				else if( *(point_below+1)== 0 && *(point+1)==0){
					*point = 0;
					*(point+map_width*2+1) = 1;
					*(active_map_buffer+i/chunk_size*map_width/chunk_size+(j+1)/chunk_size)=1;
					if(i%chunk_size == 0){
						*(active_map_buffer+(i-1)/chunk_size*map_width/chunk_size+j/chunk_size) = 1;
					}
					if(j%chunk_size == 0){
						*(active_map_buffer+(i-1)/chunk_size*map_width/chunk_size+(j-1)/chunk_size) = 1;
						*(active_map_buffer+(i-1)/chunk_size*map_width/chunk_size+(j+1)/chunk_size) = 1;
						*(active_map_buffer+(i)/chunk_size*map_width/chunk_size+(j-1)/chunk_size) = 1;
						*(active_map_buffer+(i)/chunk_size*map_width/chunk_size+(j+1)/chunk_size) = 1;
					}
					
				}
				else if(*(point_below-1) == 0&& *(point-1)==0){
					*point = 0;
					*(point+map_width*2-1) = 1;
					*(active_map_buffer+i/chunk_size*map_width/chunk_size+(j-1)/chunk_size)=1;
					if(i%chunk_size == 0){
						*(active_map_buffer+(i-1)/chunk_size*map_width/chunk_size+j/chunk_size) = 1;
					}
					if(j%chunk_size == 0){
						*(active_map_buffer+(i-1)/chunk_size*map_width/chunk_size+(j-1)/chunk_size) = 1;
						*(active_map_buffer+(i-1)/chunk_size*map_width/chunk_size+(j+1)/chunk_size) = 1;
						*(active_map_buffer+(i)/chunk_size*map_width/chunk_size+(j-1)/chunk_size) = 1;
						*(active_map_buffer+(i)/chunk_size*map_width/chunk_size+(j+1)/chunk_size) = 1;
					}
				}
			}
		}
	}
}

