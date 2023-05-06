(defn main [] :int
    (SDL_Init SDL_INIT_VIDEO)
    (var window :SDL_Renderer*
      (SDL_CreateWindow "Little Line"
        100 100 75 75
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
            )
        )

        (SDL_SetRenderDrawColor renderer 0 0 0 255)
        (SDL_RenderClear renderer)

        (SDL_SetRenderDrawColor renderer 125 200 25 255)

        (var x :int 50)
        (var y :int 50)
        (while (> x 0)
          (SDL_RenderDrawPoint renderer (+ 10 x) (+ 10 y))
          (set x (- x 1))
          (set y (- y 1)))

        (SDL_RenderPresent renderer)
    )

    (SDL_Quit)

    0)