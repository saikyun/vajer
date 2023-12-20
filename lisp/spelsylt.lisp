(declare SDL_RenderPresent [:SDL_Renderer* -> :void])
(declare SDL_SetRenderDrawColor [:SDL_Renderer* :int :int :int :int -> :void])
(declare SDL_RenderClear [:SDL_Renderer* -> :void])
(declare SDL_Quit [-> :void])
(declare SDL_CreateRenderer [:SDL_Window* :int :int -> :SDL_Renderer*])
(declare SDL_CreateWindow [[:char] :int :int :int :int :int -> :SDL_Window*])
(declare malloc [:int -> :void*])
(declare strchr [[:int] :int -> [:int]])
(declare memset [?T :int :int -> :void*])
(declare srand [:int -> :void])
(declare time [:int -> :int])
(declare rand [-> :int])
(declare SDL_Init [:int -> :int])
(declare SDL_PollEvent [[:SDL_Event] -> :int])
(declare SDL_RenderDrawPoint [:SDL_Renderer* :int :int -> :void])
(declare SDL_RenderSetScale [:SDL_Renderer* :int :int -> :void])
(declare SDL_GetTicks [-> :int])

(defstruct Point {x :int y :int})

(var width 200)
(var height 200)

(declare-var renderer :SDL_Renderer*)
(declare-var data [:int])

(defn random_dir2
  [width height x y]
  (var res (% (rand) 4))
  (if (== res 0)
    {x x,       y (- y 1)}
    (if (== res 1)
      {x (+ x 1), y y}
      (if (== res 2)
        {x x,       y (+ y 1)}
        {x (- x 1), y y}))))

(var cursor_x 50)
(var cursor_y 50)

(defn spawn
  [data width x y]
  (put data (+ x (* width y)) 1))

(defn draw_cursor
  [renderer x y]
  (SDL_SetRenderDrawColor renderer 0 255 0 255)
  (var yy -5)
  (while (<= yy 5)
    (do
      (if (|| (< yy -1) (> yy 1))
        (SDL_RenderDrawPoint renderer x (+ y yy)))
      (set yy (+ yy 1))))
  (var xx -5)
  (while (<= xx 5)
    (do
      (if (|| (< xx -1) (> xx 1))
        (SDL_RenderDrawPoint renderer (+ x xx) y))
      (set xx (+ xx 1)))))

(defn point_to_i
  [width point]
  (+ (get point x) (* (get point y) width)))

(defn get_point
  [data width point]
  (in data (point_to_i width point)))

(defn set_px [data width height point n]
  (var x (get point x))
  (var y (get point y))
  (if (&& (&& (<= 0 x) (< x width))
          (&& (<= 0 y) (< y height)))
    (put data (point_to_i width point) n)))

(defn update [data width height]
  (var y 0)
  (while (< y height)
    (do
      (var x 0)
      (while (< x width)
        (do
          (var pos (+ (* width y) x))
          (var kind (in data pos))
          (var lul (random_dir2 width height x y))
          (if (|| (&& (== kind 1) (<= 59 (% (rand) 100)))
                  (&& (== kind 2) (<= 159 (% (rand) 200))))
            (do
              (var new_point (random_dir2 width height x y))
              (var target_n (get_point data width new_point))
              (if (== target_n 0)
                (set_px data width height new_point kind)
                (if (!= target_n kind)
                  (set_px data width height new_point 0)))))

          (set x (+ x 1))))
      (set y (+ y 1)))))

(var quit 0)

(declare-struct SDL_Event {type :int, key.keysym.scancode :int})

(defn main_loop
  []
  (var e (cast :SDL_Event {}))
  (var res (SDL_PollEvent (ref e)))
  (if (!= 0 res)
    (do
      (if (== (get e type) SDL_QUIT)
        (set quit 1))

      (if (== (get e type) SDL_KEYDOWN)
        (do
          (if (== (get e key.keysym.scancode) SDL_SCANCODE_W)
            (do (set cursor_y (- cursor_y 1)) 0))
          (if (== (get e key.keysym.scancode) SDL_SCANCODE_S)
            (do (set cursor_y (+ cursor_y 1)) 0))
          (if (== (get e key.keysym.scancode) SDL_SCANCODE_A)
            (do (set cursor_x (- cursor_x 1)) 0))
          (if (== (get e key.keysym.scancode) SDL_SCANCODE_D)
            (do (set cursor_x (+ cursor_x 1)) 0))
          (if (== (get e key.keysym.scancode) SDL_SCANCODE_SPACE)
            (do (spawn data width cursor_x cursor_y) 0))
          0))))

  (update data width height)

  (SDL_SetRenderDrawColor renderer 0 0 0 255)
  (SDL_RenderClear renderer)

  (var y 0)
  (while (< y height)
    (do
      (var x 0)
      (while (< x width)
        (do
          (var kind (in data (+ (* width y) x)))
          (if (== kind 0)
            (SDL_SetRenderDrawColor renderer 100 0 0 255))

          (if (== kind 1)
            (do
              (SDL_SetRenderDrawColor renderer 0 100 0 255)))

          (if (== kind 2)
            (do
              (SDL_SetRenderDrawColor renderer 0 0 100 255)))

          (SDL_RenderDrawPoint renderer x y)
          (set x (+ x 1))))
      (set y (+ y 1))))

  (draw_cursor renderer cursor_x cursor_y)

  (SDL_RenderPresent renderer))

(defn main
  []
  (srand (time 0))

  (SDL_Init SDL_INIT_VIDEO)

  (var size (* width height))

  (var window (SDL_CreateWindow "Spelsylt" 100 100 (* 4 width) (* 4 height) SDL_WINDOW_OPENGL))
  (set renderer (SDL_CreateRenderer window -1 SDL_RENDERER_ACCELERATED))
  (SDL_RenderSetScale renderer 4 4)

  (set data (cast [:int] (malloc (* (sizeof int) size))))

  (var yy 0)
  (while (< yy height)
    (do
      (var xx 0)
      (while (< xx width)
        (do
          (put data (+ (* width yy) xx) 0)
          (set xx (+ xx 1))))
      (set yy (+ yy 1))))

  (put data (+ 190 (* height 190)) 2)

  (while (== quit 0)
    (main_loop))

  (SDL_Quit)

  0)