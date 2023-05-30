(var gold  0)
(var dead  0)

(declare SDL_RenderPresent [:SDL_Renderer* -> :void])
(declare SDL_SetRenderDrawColor [:SDL_Renderer* :int :int :int :int -> :void])
(declare SDL_RenderClear [:SDL_Renderer* -> :void])
(declare SDL_Quit [-> :void])
(declare printf [[:char] -> :void])
(declare SDL_CreateRenderer [:SDL_Window* :int :int -> :SDL_Renderer*])
(declare SDL_CreateWindow [[:char] :int :int :int :int :int -> :SDL_Window*])
(declare malloc [?T :int -> ?T])
(declare strchr [[:char] :int -> [:char]])
(declare memset [?T :int :int -> :void*])
(declare srand [:int -> :void])
(declare time [:int -> :int])
(declare rand [-> :int])
(declare SDL_Init [:int -> :int])
(declare SDL_PollEvent [:SDL_Event* -> :int])
(declare SDL_RenderDrawPoint [:SDL_Renderer* :int :int -> :void])

(defn draw_symbol
  [renderer
   symbols
   symbol_i
   x
   y]
  (var symbol (in symbols symbol_i))
  (var w 8)
  (var h 7)
  (declare c :char)
  (var c '.')
  (var i 0)
  (var j 0)
  (while (< j h)
  (do
    (while (< i w)
    (do
      (set c (in symbol (+ i (* w j))))
      (if (== *c '.')
        (SDL_RenderDrawPoint renderer (+ x i) (+ y j))
      )
      (set i (+ i 1))
    ))
    (set i 0)
    (set j (+ j 1)))
  )
)

(defn move_boulder
  [map 
   mapw 
   maph 
   from ] 
  (var to  (+ from mapw))

  (var lul  0)
  (while (< lul 1)
  (do
    (if (&& (== 0 (in map to))
        (< to (* mapw maph)))
      (do
        (printf "YOU DIED\n")
        (set dead 1)))
    (set lul 1)))

  (put map from 1)
  (put map to 4)
)

(defn move
  [map 
   from 
   to ] 
  (var thing  (in map to))

  (var lul  0)
  (while (< lul 1)
  (do
    (if (== 4 thing)
      (printf "boulder! %d\n" gold)
      (do
        (put map from 1)
        (put map to 0)
      )
    )
    (set lul 1)))
  
  (if (== 3 thing)
    (do
      (set gold (+ gold 1))
      (printf "gold! %d\n" gold)
    )
  ))

(defn move_up
  [map 
   mapw 
   maph ] 

#  (var rrr  (strchr map 0))
#  (var pos  (- rrr map))
#  (if (>= (- pos mapw) 0)
#    (move map pos (- pos mapw)))
    
    )

(defn move_left
  [map 
   mapw 
   maph ] 
#  (var rrr  (strchr map 0))
#  (var pos  (- rrr map))
#  (move map pos (- pos 1))
)

(defn move_right
  [map 
   mapw 
   maph ] 
#  (var rrr  (strchr map 0))
#  (var pos  (- rrr map))
#  (move map pos (+ pos 1))
)

(defn move_down
  [map 
   mapw 
   maph ]
#  (var rrr  (strchr map 0))
#  (var pos  (- rrr map))
#  (move map pos (+ pos mapw))
)

(defn main [] 
  (srand (time 0))

  (var at 
" ..... 
.     .
. ... .
. . . .
. .....
.      
 ....  
")

  (var blank 
"        
        
        
        
        
        
        
        
")


  (var dot 
"       
   .   
.    . 
       
   .   
     . 
 .     
")


  (var goldsym 
"   .   
  ...  
 . . . 
  ..   
   ..  
 . . . 
  ...  
")

  (var boulder 
"       
  ...  
 .   . 
 .   . 
 .   . 
 .   . 
  ...  
")

  (var symbols_list (malloc [:char*] (* 5 (sizeof char*))))

  (put symbols_list 0 at)
  (put symbols_list 1 blank)
  (put symbols_list 2 dot)
  (put symbols_list 3 goldsym)
  (put symbols_list 4 boulder)

  (var tilesize  8)
  (var mapw  8)
  (var maph  8)
  (var maplen  (* mapw maph))
  (var map (malloc [:char] (+ 1 maplen)))
  (memset map 2 maplen)
  (put map maplen '\0')

  (printf "map: %s\n" map)

  (var i  0)
  (while (< i maplen)
  (do
    (if (< 15 (% (rand) 20))
      (put map i 3)
    )
    (set i (+ i 1))
  ))

  (var i2  0)
  (while (< i2 maplen)
  (do
    (if (< 15 (% (rand) 20))
      (put map i2 4)
    )
    (set i2 (+ i2 1))
  ))

  (put map 5 0)

  (SDL_Init SDL_INIT_VIDEO)
  (var window
    (SDL_CreateWindow "Little Line"
      100 100 (* mapw tilesize) (* maph tilesize)
      SDL_WINDOW_OPENGL))
  
  (var renderer (SDL_CreateRenderer window -1 SDL_RENDERER_ACCELERATED))

  (printf "window: %d\n" window)
  (printf "renderer: %d\n" renderer)
  
  (var quit  1000000000)
  (var e :SDL_Event)
  (while (> quit 0)
  (do
    (set quit (- quit 1 ))
    (var res  (SDL_PollEvent &e))
    (if (!= res 0)
      (do
        (if (== e.type SDL_QUIT)
          (set quit 0))
        (if (&& (== 0 dead)
                (== e.type SDL_KEYDOWN))
          (do
          1
#            (if (== e.key.keysym.sym SDLK_w)
#              (move_up map mapw maph))
#            (if (== e.key.keysym.sym SDLK_s)
#              (move_down map mapw maph))
#            (if (== e.key.keysym.sym SDLK_a)
#              (move_left map mapw maph))
#            (if (== e.key.keysym.sym SDLK_d)
#              (move_right map mapw maph))
              )
        )
      )
    )

    (SDL_SetRenderDrawColor renderer 0 0 0 255)
    (SDL_RenderClear renderer)

    (SDL_SetRenderDrawColor renderer 125 200 25 255)

    # (draw_symbol renderer at 10 10)

    (var x  0)
    (var y  0)
    (while (< y maph)
    (do
      (while (< x mapw)
      (do
        (var pos  (+ x (* y mapw)))
        (if (&& (== (in map pos) 4)
                  (|| (== (in map (+ pos mapw)) 0)
                      (== (in map (+ pos mapw)) 1)))
#          (move_boulder map mapw maph pos)
1
        )
        (draw_symbol renderer symbols_list (in map pos) (* x tilesize) (* y tilesize))
        (set x (+ x 1))
      ))
      (set x 0)
      (set y (+ y 1))

      (SDL_RenderPresent renderer))
  )))

  (SDL_Quit)

  0)