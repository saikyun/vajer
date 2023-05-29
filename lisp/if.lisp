(declare malloc [:int -> :void*])
(declare printf [[:char] -> :void])

(defn main []
  (if 1 (malloc 1))

  (if 0
    (malloc 1)
    (malloc 1337))

  (var lul (if 0 "a" "b"))

  (if 0 (printf "hehe"))

  (var dead 0)

  (if 0
    (do
      (printf "YOU DIED\n")
      (set dead 1)))


  (var map "")
  
  (if (== 4 dead)
    (printf "boulder!\n")
    (do
      (put map 0 'x')
      (put map 1 'y')
    ))

  #(declare e :SDL_Event)
  (var e 0)

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
  
  
  0)