(defn starter
    [] :int
    (printf "lule\n")
    0
    )

(defn main [] :int
    (starter)
    (SDL_Init 0)
    (SDL_Quit)
    0)