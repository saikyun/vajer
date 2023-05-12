(var gold :int 0)
(var dead :int 0)

(declare SDL_RenderPresent :void)
(declare SDL_SetRenderDrawColor :void)
(declare SDL_RenderClear :void)
(declare SDL_Quit :void)
(declare printf :void)
(declare srand :void)

(defn draw-symbol
  [renderer :SDL_Renderer*
   symbols  :char**
   symbol_i   :char
   x        :int
   y        :int] :int
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
      (do
        (SDL_RenderDrawPoint renderer (+ x i) (+ y j))
        0)
      )
      (set i (+ i 1))
    )
    (set i 0)
    (set j (+ j 1))
  )
  0
)

(defn move-boulder
  [map :char*
   mapw :int
   maph :int
   from :int] :int
  (var to :int (+ from mapw))

  (var lul :int 0)
  (while (< lul 1)
    (if (&& (== 0 (in map to))
        (< to (* mapw maph))
    )
    (do
      (printf "YOU DIED\n")
      (set dead 1)
      0))
    (set lul 1))

  (put map from 1)
  (put map to 4)

  0
)

(defn move
  [map :char*
   from :int
   to :int] :int
  (var thing :int (in map to))

  (var lul :int 0)
  (while (< lul 1)
    (if (== 4 thing)
      (do
        (printf "boulder! %d\n" gold)
        0
      )
      (do
        (put map from 1)
        (put map to 0)
        0
      )
      )
    (set lul 1))
  
  (if (== 3 thing)
    (do
    (set gold (+ gold 1))
    (printf "gold! %d\n" gold)
    0
    )
    
    0
  ))

(defn move-up
  [map :char*
   mapw :int
   maph :int] :int
  (var rrr :char* (strchr map 0))
  (var pos :int (- rrr map))
  (if (>= (- pos mapw) 0)
    (do
      (move map pos (- pos mapw))
      0))
)

(defn move-left
  [map :char*
   mapw :int
   maph :int] :int
  (var rrr :char* (strchr map 0))
  (var pos :int (- rrr map))
  (move map pos (- pos 1))
  0
)

(defn move-right
  [map :char*
   mapw :int
   maph :int] :int
  (var rrr :char* (strchr map 0))
  (var pos :int (- rrr map))
  (move map pos (+ pos 1))
  0
)

(defn move-down
  [map :char*
   mapw :int
   maph :int] :int
  (var rrr :char* (strchr map 0))
  (var pos :int (- rrr map))
  (move map pos (+ pos mapw))
  0
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

  (var boulder :char*
"       
  ...  
 .   . 
 .   . 
 .   . 
 .   . 
  ...  
")

  (var symbols :char** (malloc (* 5 (sizeof char*))))

  (set *symbols at)
  (var ref :char** (+ symbols 1))
  (set *ref blank)
  (set ref (+ symbols 2))
  (set *ref dot)
  (set ref (+ symbols 3))
  (set *ref gold)
  (set ref (+ symbols 4))
  (set *ref boulder)

  (var tilesize :int 8)
  (var mapw :int 8)
  (var maph :int 8)
  (var maplen :int (* mapw maph))
  (var map :char* (malloc (+ 1 maplen)))
  (memset map 2 maplen)
  (put map maplen '\0')

  (printf "map: %s\n" map)

  (var i :int 0)
  (while (< i maplen)
    (if (< 15 (% (rand) 20))
    (do
      (put map i 3)
      0
    )
    )
    (set i (+ i 1))
  )

  (var i2 :int 0)
  (while (< i2 maplen)
    (if (< 15 (% (rand) 20))
    (do
      (put map i2 4)
      0)
    )
    (set i2 (+ i2 1))
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
            (do
              (set quit 0)
              0))
            (if (&& (== 0 dead)
                    (== e.type SDL_KEYDOWN))
              (do
                (if (== e.key.keysym.sym SDLK_w)
                  (move_up map mapw maph))
                (if (== e.key.keysym.sym SDLK_s)
                  (move_down map mapw maph))
                (if (== e.key.keysym.sym SDLK_a)
                  (move_left map mapw maph))
                (if (== e.key.keysym.sym SDLK_d)
                  (move_right map mapw maph))
                  0)
            )
            0
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
          (var pos :int (+ x (* y mapw)))
          (if (&& (== (in map pos) 4)
                   (|| (== (in map (+ pos mapw)) 0)
                       (== (in map (+ pos mapw)) 1)))
            (move_boulder map mapw maph pos)
          )
          (draw_symbol renderer symbols (in map pos) (* x tilesize) (* y tilesize))
          (set x (+ x 1))
        )
        (set x 0)
        (set y (+ y 1)))

      (SDL_RenderPresent renderer)
  )

  (SDL_Quit)

  0)