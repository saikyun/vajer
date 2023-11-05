(declare printf [[:char] -> :void])
(declare malloc [:int -> :void*])
(defstruct Cat {data [:int]})

(defn get_pos
  [cat]
  (var datta2 (get cat data))
  0)

(defn wat [cat]
  (var pos (get_pos cat))
  (var n (get cat data))
  0)

(defn main []
  (var cat (cast :Cat {data (cast [:int] (malloc (* (sizeof int) 10)))}))
  (wat cat)

  0)