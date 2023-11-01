(declare printf [[:char] -> :void])
(defstruct Cat {name :int})

(defn main []
    (var cat (cast :Cat {name 5}))
    (:= cat name (if 0 10 5))
    (var n (get cat name))

    #(printf n)
    0)