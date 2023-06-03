(declare malloc [:int -> :void*])
(declare cast [:symbol ?T2 -> ?T])

(defn main []
    (var x (cast :char* (malloc 10)))
    0)