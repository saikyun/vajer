(var gold :int 0)

(defn draw-symbol
  [renderer :SDL_Renderer*
   symbols  :char**
   symbol_i   :char
   x        :int
   y        :int] :void
  (var symbol :char* (in symbols symbol_i))
  (var w :int 8)
  (var h :int 7)
  (var c :char*)
  (var i :int 0)
  (var j :int 0)
  (while (< j h)
    (while (< i w)
      (set c (+ symbol (+ i (* w j))))
      (if (== *c '.')
        (SDL_RenderDrawPoint renderer (+ x i) (+ y j)))
      (set i (+ i 1))
    )
    (set i 0)
    (set j (+ j 1))
  )
)

(defn move
  [map :char*
   from :int
   to :int] :int
  (var thing :int (in map to))
  (put map from 1)
  (put map to 0)
  
  (if (== 3 thing)
    (do
    (set gold (+ gold 1))
    (printf "gold! %d\n" gold)
    )
    0
  ))

(defn move-up
  [map :char*
   mapw :int
   maph :int] :int
  (var pos :int (- (strchr map 0) map))
  (if (>= (- pos mapw) 0)
    (do
      (move map pos (- pos mapw))
      0))
)

(defn move-left
  [map :char*
   mapw :int
   maph :int] :int
  (var pos :int (- (strchr map 0) map))
  (move map pos (- pos 1))
  0
)

(defn move-right
  [map :char*
   mapw :int
   maph :int] :int
  (var pos :int (- (strchr map 0) map))
  (move map pos (+ pos 1))
  0
)

(defn move-down
  [map :char*
   mapw :int
   maph :int] :void
  (var pos :int (- (strchr map 0) map))
  (move map pos (+ pos mapw))
)

(defn main [] :int
  (srand (time NULL))

  (var at :char*
" ..... 
.     .
. ... .
. . . .
. .....
.      
 ....  
")

  (var blank :char*
"        
        
        
        
        
        
        
        
")


  (var dot :char*
"       
   .   
.    . 
       
   .   
     . 
 .     
")


  (var gold :char*
"   .   
  ...  
 . . . 
  ..   
   ..  
 . . . 
  ...  
")

  (var symbols :char** (malloc (* 4 (sizeof char*))))

  (set *symbols at)
  (var ref :char** (+ symbols 1))
  (set *ref blank)
  (set ref (+ symbols 2))
  (set *ref dot)
  (set ref (+ symbols 3))
  (set *ref gold)

  (var tilesize :int 8)
  (var mapw :int 8)
  (var maph :int 8)
  (var maplen :int (* mapw maph))
  (var map :char* (malloc (+ 1 maplen)))
  (memset map 2 maplen)
  (put map maplen '\0')

  (printf "map: %s\n" map)

  (var i :int)
  (while (< i maplen)
    (if (< 15 (% (rand) 20))
      (put map i 3)
    )
    (set i (+ i 1))
  )

  (put map 5 0)

  (SDL_Init SDL_INIT_VIDEO)
  (var window :SDL_Renderer*
    (SDL_CreateWindow "Little Line"
      100 100 (* mapw tilesize) (* maph tilesize)
      SDL_WINDOW_OPENGL))
  
  (var renderer :SDL_Renderer* (SDL_CreateRenderer window -1 SDL_RENDERER_ACCELERATED))

  (printf "window: %d\n" window)
  (printf "renderer: %d\n" renderer)
  
  (var quit :int 1000000000)
  (var e :SDL_Event)
  (while (> quit 0)
      (set quit (- quit 1 ))
      (var res :int (SDL_PollEvent &e))
      (if (!= res 0)
          (do
            (if (== e.type SDL_QUIT)
              (set quit 0))
            (if (== e.type SDL_KEYDOWN)
              (do
                (if (== e.key.keysym.sym SDLK_w)
                  (move_up map mapw maph))
                (if (== e.key.keysym.sym SDLK_s)
                  (move_down map mapw maph))
                (if (== e.key.keysym.sym SDLK_a)
                  (move_left map mapw maph))
                (if (== e.key.keysym.sym SDLK_d)
                  (move_right map mapw maph)))
            )
          )
      )

      (SDL_SetRenderDrawColor renderer 0 0 0 255)
      (SDL_RenderClear renderer)

      (SDL_SetRenderDrawColor renderer 125 200 25 255)

      # (draw_symbol renderer at 10 10)

      (var x :int 0)
      (var y :int 0)
      (while (< y maph)
        (while (< x mapw)
          (draw_symbol renderer symbols (in map (+ x (* y mapw))) (* x tilesize) (* y tilesize))
          (set x (+ x 1))
        )
        (set x 0)
        (set y (+ y 1)))

      (SDL_RenderPresent renderer)
  )

  (SDL_Quit)

  0)