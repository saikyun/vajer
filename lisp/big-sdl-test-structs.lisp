(var gold 0)
(var dead 0)

(declare SDL_RenderPresent [:SDL_Renderer* -> :void])
(declare SDL_SetRenderDrawColor [:SDL_Renderer* :int :int :int :int -> :void])
(declare SDL_RenderClear [:SDL_Renderer* -> :void])
(declare SDL_Quit [-> :void])
(declare printf [[:char] -> :void])
(declare SDL_CreateRenderer [:SDL_Window* :int :int -> :SDL_Renderer*])
(declare SDL_CreateWindow [[:char] :int :int :int :int :int -> :SDL_Window*])
(declare malloc [:int -> :void*])
(declare strchr [[:int] :int -> [:int]])
(declare memset [?T :int :int -> :void*])
(declare srand [:int -> :void])
(declare time [:int -> :int])
(declare rand [-> :int])
(declare SDL_Init [:int -> :int])
(declare SDL_PollEvent [:SDL_Event* -> :int])
(declare SDL_RenderDrawPoint [:SDL_Renderer* :int :int -> :void])

(defn draw_symbol
  [renderer
   map
   symbols
   symbol_i
   x
   y]
  (var symbol (in symbols symbol_i))
  (var w (get map width))
  (var h (get map height))
  (declare c :char)
  (var c '1')
  (var i 0)
  (var j 0)
  (while (< j h)
    (do
      (while (< i w)
        (do
          (set c (in symbol (+ i (* w j))))
          (if (== c '.')
            (SDL_RenderDrawPoint renderer (+ x i) (+ y j)))
          (set i (+ i 1))))
      (set i 0)
      (set j (+ j 1)))))

(defstruct Map
   {data   [:int]
    width  :int
    height :int})

(defn move_boulder
  [mapre from]
  (var to (+ from (get mapre width)))
  (var lul 0)
  (while (< lul 1)
    (do
      (if (&& #(== 0 (in (get mapre data) to))
      1
              (< to (* (get mapre width) (get mapre height))))
        (do
          (printf "YOU DIED\n")
          (set dead 1)))
      (set lul 1)))

  #(put (get mapre data) from 1)
  #(put (get mapre data) to 4)
  )

(defn move
  [map from to]
  (var thing (in map to))

  (var lul 0)
  (while (< lul 1)
    (do
      (if (== 4 thing)
        (printf "boulder!\n")
        (do
          (put map from 1)
          (put map to 0)))

      (set lul 1)))

  (if (== 3 thing)
    (do
      (set gold (+ gold 1))
      (printf "gold!\n"))))

(defn get_pos
  [map mapw maph el]
  (var i 0)
  (var res -1)
  (while (< i (* mapw maph))
    (do
      (if (== (in map i) el)
        (set res i))
      (set i (+ i 1))))
  res)

(defn move_up
  [map mapw maph]
  (var pos (get_pos map mapw maph 0))
  (if (>= (- pos mapw) 0)
    (move map pos (- pos mapw))))

(defn move_down
  [map mapw maph]
  (var pos (get_pos map mapw maph 0))
  (move map pos (+ pos mapw)))

(defn move_left
  [map mapw maph]
  (var pos (get_pos map mapw maph 0))
  (move map pos (- pos 1)))

(defn move_right
  [map mapw maph]
  (var pos (get_pos map mapw maph 0))
  (move map pos (+ pos 1)))

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

  (var symbols_list (cast [[:char]] (malloc (* 5 (sizeof char*)))))

  (put symbols_list 0 at)
  (put symbols_list 1 blank)
  (put symbols_list 2 dot)
  (put symbols_list 3 goldsym)
  (put symbols_list 4 boulder)




  (var map_s (cast :Map {data (cast [:char] (malloc (* 8 8)))
                         width 8
                         height 8}))

  (var tilesize 8)
  (var maplen (* (get map_s width) (get map_s height)))

  # (var map (cast [:int] (malloc (* (sizeof int) (+ 1 maplen)))))
  (memset (get map_s data) 0 maplen)
  (put (get map_s data) maplen 0)

  (printf "map\n")

  (var i 0)
  (while (< i maplen)
    (do
      (if (< 15 (% (rand) 20))
        (put (get map_s data) i 3)
        (put (get map_s data) i 2))
      (set i (+ i 1))))

  (var i2 0)
  (while (< i2 maplen)
    (do
      (if (< 15 (% (rand) 20))
        (put (get map_s data) i2 4))
      (set i2 (+ i2 1))))

  (put (get map_s data) 5 0)

  (SDL_Init SDL_INIT_VIDEO)
  (var window
       (SDL_CreateWindow "Little Line"
                         100 100 (* (get map_s width) tilesize) (* (get map_s height) tilesize)
                         SDL_WINDOW_OPENGL))

  (var renderer (SDL_CreateRenderer window -1 SDL_RENDERER_ACCELERATED))

  (printf "window:\n")
  (printf "renderer:\n")

  (var quit 1000000000)
  (declare e :SDL_Event)
  (var e NULL)
  (while (> quit 0)
    (do
      (set quit (- quit 1))
      (var res (SDL_PollEvent &e))
      (if (!= res 0)
        (do
          (if (== e.type SDL_QUIT)
            (set quit 0))
          (if (&& (== 0 dead)
                  (== e.type SDL_KEYDOWN))
            (do
              #(if (== e.key.keysym.sym SDLK_w)
              #  (move_up map mapw maph))
              #(if (== e.key.keysym.sym SDLK_s)
              #  (move_down map mapw maph))
              #(if (== e.key.keysym.sym SDLK_a)
              #  (move_left map mapw maph))
              #(if (== e.key.keysym.sym SDLK_d)
              #  (move_right map mapw maph))
              
                ))))

      (SDL_SetRenderDrawColor renderer 0 0 0 255)
      (SDL_RenderClear renderer)

      (SDL_SetRenderDrawColor renderer 125 200 25 255)

      (var x 0)
      (var y 0)
      (while (< y (get map_s height))
        (do
          (while (< x (get map_s width))
            (do
              (var pos  (+ x (* y (get map_s width))))
              (if (&& (== (in (get map_s data) pos) 4)
                      (|| (== (in (get map_s data) (+ pos mapw)) 0)
                          (== (in (get map_s data) (+ pos mapw)) 1)))
                (move_boulder map_s pos))
              (draw_symbol renderer map_s symbols_list (in (get map_s data) pos) (* x tilesize) (* y tilesize))
              (set x (+ x 1))))
          (set x 0)
          (set y (+ y 1))))
      (SDL_RenderPresent renderer)))

  (SDL_Quit)

  0)