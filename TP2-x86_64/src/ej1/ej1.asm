; /** defines bool y puntero **/
%define NULL 0
%define TRUE 1
%define FALSE 0

section .data

section .text

global string_proc_list_create_asm
global string_proc_node_create_asm
global string_proc_list_add_node_asm
global string_proc_list_concat_asm


extern malloc
extern free
extern str_concat

; ================================================
; string_proc_list_create_asm: Crea una lista vacía
; Retorna: RAX = puntero a la lista (o NULL si falla)
; ================================================
string_proc_list_create_asm:
    push rbp                ; Preserva el base pointer
    mov rbp, rsp            ; Establece nuevo frame pointer
    
    ; Preparar argumento para malloc (16 bytes: 2 punteros)
    mov rdi, 16             ; Tamaño de la estructura: 2 punteros (next, prev)
    call malloc             ; Llama a malloc(16)
    
    ; Verificar si malloc falló (RAX == NULL)
    test rax, rax           ; ¿RAX es cero?
    jz .end                 ; Si falló, saltar al final
    
    ; Inicializar la lista (ambos punteros a NULL)
    mov qword [rax], NULL   ; next = NULL
    mov qword [rax+8], NULL ; prev = NULL
    
.end:
    pop rbp                 ; Restaura el base pointer
    ret                     ; Retorna (RAX contiene la lista o NULL)

; ================================================
; string_proc_node_create_asm: Crea un nodo de la lista
; Parámetros:
;   - DIL (parte baja de RDI): tipo (8 bits)
;   - RSI: puntero al hash (string)
; Retorna: RAX = puntero al nodo (o NULL si falla)
; ================================================
string_proc_node_create_asm:
    push rbp
    mov rbp, rsp
    push rbx                ; Preserva RBX (registro no volátil)
    push r12                ; Preserva R12 (registro no volátil)
    
    ; Verificar si el hash es NULL
    test rsi, rsi           ; ¿RSI (hash) es NULL?
    jz .return_null         ; Si es NULL, retornar NULL
    
    ; Guardar parámetros en registros preservados
    mov bl, dil             ; BL = tipo (8 bits, parte baja de RBX)
    mov r12, rsi            ; R12 = hash (puntero)
    
    ; Llamar a malloc(32 bytes)
    mov rdi, 32             ; Tamaño del nodo: 32 bytes (2 punteros + tipo + hash)
    call malloc
    
    ; Verificar si malloc falló
    test rax, rax
    jz .return_null
    
    ; Inicializar el nodo
    mov qword [rax], NULL   ; next = NULL
    mov qword [rax+8], NULL ; prev = NULL
    mov byte [rax+16], bl    ; type = BL (8 bits)
    mov qword [rax+24], r12  ; hash = R12 (puntero)
    
    jmp .end
    
.return_null:
    xor rax, rax            ; RAX = NULL (fallo)
    
.end:
    pop r12                 ; Restaura R12
    pop rbx                 ; Restaura RBX
    pop rbp
    ret

; ================================================
; string_proc_list_add_node_asm: Añade un nodo a la lista
; Parámetros:
;   - RDI: puntero a la lista
;   - RSI: tipo (8 bits)
;   - RDX: puntero al hash
; Retorna: RAX = puntero al nodo añadido (o NULL si falla)
; ================================================
string_proc_list_add_node_asm:
    push rbp
    mov rbp, rsp
    push r12                ; Preserva registros no volátiles
    push r13
    push r14
    push r15
    
    ; Guardar parámetros en registros preservados
    mov r12, rdi            ; R12 = lista
    mov r13, rdx            ; R13 = hash
    mov r14, rsi            ; R14 = tipo (64 bits, aunque solo usamos 8)
    
    ; Verificar si la lista es NULL
    test r12, r12
    jz .return_null
    
    ; Crear el nodo (type en R14B, hash en R13)
    mov rdi, r14            ; type (primer argumento)
    mov rsi, r13            ; hash (segundo argumento)
    call string_proc_node_create_asm
    
    ; Verificar si la creación falló
    test rax, rax
    jz .return_null
    
    ; Guardar el nodo en R15
    mov r15, rax            ; R15 = nuevo nodo
    
    ; Verificar si la lista está vacía (lista->first == NULL)
    cmp qword [r12], NULL
    jne .list_not_empty
    
    ; Caso: lista vacía
    mov [r12], r15          ; lista->first = nuevo_nodo
    mov [r12+8], r15        ; lista->last = nuevo_nodo
    jmp .end
    
.list_not_empty:
    ; Caso: lista no vacía
    mov rax, [r12+8]        ; RAX = lista->last
    mov [rax], r15          ; lista->last->next = nuevo_nodo
    mov [r15+8], rax        ; nuevo_nodo->prev = lista->last
    mov [r12+8], r15        ; lista->last = nuevo_nodo
    
.end:
    mov rax, r15            ; Retorna el nodo creado
    jmp .cleanup
    
.return_null:
    xor rax, rax            ; Retorna NULL
    
.cleanup:
    pop r15                 ; Restaura registros
    pop r14
    pop r13
    pop r12
    pop rbp
    ret

; ================================================
; string_proc_list_concat_asm: Concatena hashes de nodos con un tipo específico
; Parámetros:
;   - RDI: puntero a la lista
;   - SIL: tipo a filtrar (8 bits)
;   - RDX: hash adicional (opcional)
; Retorna: RAX = string concatenado (o NULL si falla)
; ================================================
string_proc_list_concat_asm:
    push rbp
    mov rbp, rsp
    push rbx                ; Registros preservados
    push r12
    push r13
    push r14
    push r15
    sub rsp, 8              ; Alineación de pila (opcional)
    
    ; Guardar parámetros
    mov rbx, rdi            ; RBX = lista
    mov r12b, sil           ; R12B = tipo (8 bits)
    mov r13, rdx            ; R13 = hash adicional
    
    ; Verificar si la lista es NULL
    test rbx, rbx
    jz .return_null
    
    ; Crear un string vacío (solo '\0')
    mov rdi, 1              ; malloc(1)
    call malloc
    test rax, rax
    jz .return_null
    
    ; Inicializar string vacío
    mov byte [rax], 0       ; *rax = '\0'
    mov r14, rax            ; R14 = string acumulado
    
    ; Iniciar iteración (R15 = primer nodo)
    mov r15, [rbx]          ; R15 = lista->first
    
.loop_start:
    ; Verificar fin de la lista (R15 == NULL)
    test r15, r15
    jz .loop_end
    
    ; Cargar el tipo del nodo actual (8 bits)
    mov al, byte [r15+16]   ; AL = nodo->type (carga 1 byte)
    cmp al, r12b            ; Comparar con el tipo buscado (R12B)
    jne .next_node          ; Saltar si no coincide
    
    ; Concatenar el hash del nodo al string acumulado
    mov rdi, r14            ; RDI = string acumulado
    mov rsi, [r15+24]       ; RSI = nodo->hash
    call str_concat          ; Llama a str_concat(acumulado, hash)
    
    ; Liberar el string anterior y guardar el nuevo
    mov rdi, r14            ; RDI = string antiguo
    mov r14, rax            ; R14 = nuevo string (retornado por str_concat)
    call free               ; Libera el string antiguo
    
.next_node:
    ; Avanzar al siguiente nodo
    mov r15, [r15]          ; R15 = R15->next
    jmp .loop_start
    
.loop_end:
    ; Verificar si hay un hash adicional para concatenar
    test r13, r13           ; ¿R13 (hash adicional) es NULL?
    jz .return_result
    
    ; Concatenar el hash adicional
    mov rdi, r13            ; RDI = hash adicional
    mov rsi, r14            ; RSI = string acumulado
    call str_concat
    
    ; Liberar el string anterior y guardar el nuevo
    mov rdi, r14
    mov r14, rax
    call free
    
.return_result:
    ; Retornar el string concatenado
    mov rax, r14
    jmp .cleanup
    
.return_null:
    xor rax, rax            ; Retorna NULL
    
.cleanup:
    add rsp, 8              ; Restaura alineación de pila
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret