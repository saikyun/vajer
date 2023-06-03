(declare malloc [:int -> :void*])

(defn draw_symbol
  [symbols]
  (in symbols 0)
  0
)

(defn main [] 
  (var symbols_list (cast [[:char]] (malloc 1)))
  (var symbols_list2 (cast [[:int]] (malloc 1)))
  (draw_symbol symbols_list)
  0)