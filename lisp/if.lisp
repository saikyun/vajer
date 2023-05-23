(declare malloc :void*)
(declare printf :void)

(defn main [] :int
  (if 1 (malloc 1))

  (if 0 (malloc 1) (malloc 1337))

  (var lul :char* (if 0 "a" "b"))

  (if 0 (printf "hehe"))

  (var dead :int 0)

  (if 0
    (do
      (printf "YOU DIED\n")
      (set dead 1)))


  (var map :char*)
    
  (if (== 4 dead)
    (printf "boulder!\n")
    (do
      (put map 0 1)
      (put map 1 0)
    ))

  (var e :int)

  (if (&& (== 0 dead)
          (== e SDL_KEYDOWN))
    (do
      (if (== e SDLK_w)
        (printf map))
      (if (== e SDLK_s)
        (printf map))
      (if (== e SDLK_a)
        (printf map))
      (if (== e SDLK_d)
        (printf map)))
  )
  
  
  1)