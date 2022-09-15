.586p

; Структура дескриптора сегмента
descr struct
	limit 	DW 0 
	base_l 	DW 0
	base_m 	DB 0
	attr_1 	DB 0
	attr_2 	DB 0
	base_h 	DB 0
descr ends

; Структура дескриптора прерывания
idescr struc
    offs_l  dw 0
    sel     dw 0
    cntr    db 0
    attr    db 0
    offs_h  dw 0
idescr ends

; Сегмент стека.
stack32 segment  para stack 'STACK'
    stack_start db  100h dup(?)
    stack_size = $-stack_start
stack32 ends

; Сегмент данных
data32 segment para 'data'
    gdt_null  descr <>
    ; дескриптор, описывающий сегмент кода для реального режима
    gdt_code16 descr <code16_size-1,0,0,98h>
    gdt_data4gb descr <0FFFFh,0,0,92h,0CFh>
    gdt_code32 descr <code32_size-1,0,0,98h,40h>
    gdt_data32 descr <data_size-1,0,0,92h,40h>
    gdt_stack32 descr <stack_size-1,0,0,92h,40h>
    gdt_video16 descr <3999,8000h,0Bh,92h> ; 0 + B 0000h + 8000h = 0B8000h 


    ; Размер таблицы GDT.
    gdt_size=$-gdt_null
    ; DF - выделить поле для псевдодескриптора (6-байт).
    pdescr    df 0

    ; Селекторы - номер (индекс начала) дескриптора в GDT.
    code16s=8
    data4gbs=16
    code32s=24
    data32s=32
    stack32s=40
    video16s=48

    ; idt - метка начала IDT
    idt label byte

    idescr_0_12 idescr 13 dup (<0,code32s,0,8Fh,0>) 

    idescr_13 idescr <0,code32s,0,8Fh,0>
    
    ;[14, 31]
    idescr_14_31 idescr 18 dup (<0,code32s,0,8Fh,0>)

    ; 10001110 - Eh - шлюз прирываний - служит для обработки прерывания.
    int08 idescr <0,code32s,0,10001110b,0> 
    int09 idescr <0,code32s,0,10001110b,0>

    ; Размер таблицы дескрипторов прерываний.
    idt_size=$-idt

    ; interruption psevdo descriptor
    ; DF - выделить поле для псевдодескриптора (6-байт).
    ipdescr df 0

    ipdescr16 dw 3FFh, 0, 0 

    mask_master db 0        
    mask_slave  db 0        
    
    asciimap   db 0, 0, 49, 50, 51, 52, 53, 54, 55, 56, 57, 48, 45, 61, 8, 0
    db 113, 119, 101, 114, 116, 121, 117, 105, 111, 112, '[', ']', 0, 0, 97, 115
    db 100, 102, 103, 104, 106, 107, 108, 59, 39, 96, 7, 92, 122, 120, 99
    db 118, 98, 110, 109, 44, 46, 47

    flag_enter_pr db 0        
	flag_shift_pr db 0
	
	start_pos 		  dd 2 * 80 * 5
    syml_pos      dd 2 * 80 * 5

    mem_pos=0 

    mem_value_pos=14 + 16
    mb_pos=30 + 2
    cursor_pos=80
    param=1Eh
	

    cursor_symb=219
    color db 00Fh ; Цвет курсора.
    
    rm_msg      db 27, '[30;42mNow in Real Mode. ', 27, '[0m$', '$'
    pm_msg_wait db 27, '[30;42mPress any button to enter protected mode!', 27, '[0m$'
    pm_msg_out  db 27, '[30;42mNow in Real Mode again! ', 27, '[0m$'
    pm_mem_count db 'Memory: '

    data_size = $-gdt_null 
data32 ends


code32 segment para public 'code' use32
    assume cs:code32, ds:data32, ss:stack32

pm_start:
    mov ax, data32s
    mov ds, ax
    mov ax, video16s
    mov es, ax
    mov ax, stack32s
    mov ss, ax
    mov eax, stack_size
    mov esp, eax

    sti ; Резрешаем (аппаратные) прерывания
    
    ; Вывод сообщения "Memory"
    mov di, mem_pos
    mov ah, param
    xor esi, esi
    xor ecx, ecx
    mov cx, 8 ; Длина строки
    print_memory_msg:
        mov al, pm_mem_count[esi]
        stosw ; al (символ) с параметром (ah) перемещается в область памяти es:di
        inc esi
    loop print_memory_msg

    ; Считаем и выводим кол-во физической памяти, выделенной dosbox'у.
    call count_memory_proc
    
    ; Цикл, пока не будет введен Enter
    proccess:
        test flag_enter_pr, 1 ; если flag = 1, то выход
    jz  proccess

    ; Выход из защищенного режима
    cli ; Запрет аппаратных маскируемые прерывания прерываний.

    db  0EAh ; jmp
    dd  offset return_rm ; offset
    dw  code16s ; selector


    ; Зашлушка для исключений.
    except_1 proc
        iret
    except_1 endp

    ; Заглушка для 13 исключения.
    ; Нужно снять со стека код ошибки.
    except_13 proc uses eax
        pop eax
        iret
    except_13 endp


    new_int08 proc uses eax 
        mov edi, cursor_pos ; поместим в edi позицию для вывода

        mov ah, color ; В ah помещаем цвет текста.
        ror ah, 1        
        mov color, ah
        mov al, cursor_symb ; Символ, который мы хотим вывести (в моем случае просто квадрат).
        stosw ; al (символ) с параметром (ah) перемещается в область памяти es:di

        mov al, 20h
        out 20h, al

        iretd ; double - 32 битный iret
    new_int08 endp

    ; uses - сохраняет контекст (push + pop)
    new_int09 proc uses eax ebx edx
        ;Порт 60h при чтении содержит скан-код последней нажатой клавиши.
        in  al, 60h ; считываем порт клавы
        cmp al, 1Ch ; сравниваем с Enter'ом

        jne print_value         
        or flag_enter_pr, 1 ; если Enter, устанавливаем флаг
        jmp exit
		

		print_value:
			cmp al, 80h
			ja exit
			
			xor ah, ah   

			xor ebx, ebx
			mov bx, ax

			mov dh, param
			mov dl, asciimap[ebx]
		
			mov ebx, syml_pos   
			cmp dl, 8
			je del_symb
			
			cmp dl, 7
			je shift_down
			
			cmp flag_shift_pr, 1
			jne print
			
			; Проверяем, что выводим букву
			cmp dl, 96
			jle print
			
			cmp dl, 123
			jge print
			
			sub dl, 32
			jmp print
		
		shift_down:
			mov dl, '*'
			cmp flag_shift_pr, 1
			je shift_up
			
			mov flag_shift_pr, 1
			jmp print
		
		shift_up:
			mov flag_shift_pr, 0
			jmp print
			
		print:
			mov es:[ebx], dx

			add ebx, 2          
			mov syml_pos, ebx
			jmp exit
		
		del_symb:
			sub ebx, 2
			mov dh, 0
			mov dl, ' '
			mov es:[ebx], dx
			mov syml_pos, ebx
			jmp exit


		exit: 

			mov al, 20h 
			out 20h, al

			iretd
    new_int09 endp

    count_memory_proc proc uses ds eax ebx
        mov ax, data4gbs ; Селектор, который указывает на дескриптор, описывающий сегмент 4 Гб.
        mov ds, ax ; На данном этапе в сегментный регистр помещается селектор data4gbs

        mov ebx,  100001h ;
        mov dl,   0AEh 

        mov ecx, 0FFEFFFFEh

        count_memory:
            mov dh, ds:[ebx]

            mov ds:[ebx], dl        

            cmp ds:[ebx], dl        
        
            jne print_memory_counter        
        
            mov ds:[ebx], dh    ; Обратно запиываем считанное значени.
            inc ebx             ; Увеличиваем счетчик.
        loop count_memory

    print_memory_counter:
        mov eax, ebx 
        xor edx, edx

        mov ebx, 100000h
        div ebx 

        mov ebx, mem_value_pos
        call print_count_memory

        ; Печать надписи Mb (мегабайты)
        mov ah, param
        mov ebx, mb_pos
        mov al, 'M'
        mov es:[ebx], ax

        mov ebx, mb_pos + 2
        mov al, 'b'
        mov es:[ebx], ax
        ret

    count_memory_proc endp

    print_count_memory proc uses ecx ebx edx

        mov ecx, 8
        mov dh, param

        print_symbol:
            mov dl, al

            and dl, 0Fh 

            cmp dl, 10

            jl to_ten ; До десяти.

            sub dl, 10

            add dl, 'A'
            jmp after_ten

        
        to_ten:
            add dl, '0'  ; Превращаем в строковое представление число.
        after_ten:
            mov es:[ebx], dx 

            ror eax, 4 
            sub ebx, 2     
        loop print_symbol

        ret
    print_count_memory endp

    code32_size = $-pm_start
code32 ends


code16 segment para public 'CODE' use16
assume cs:code16, ds:data32, ss: stack32

NewLine: 
    xor dx, dx
    mov ah, 2 
    mov dl, 13
    int 21h    
    mov dl, 10 
    int 21h
    ret

ClearScreen:
    ; Инструкция очистки экрана
    mov ax, 3
    int 10h
    ret


; Начало программы.
start:
    mov ax, data32
    ; Инициализируем DS сегментом данных.
    mov ds, ax

    ; Выводим сообщение, о том, что мы в реальном режиме.
    mov ah, 09h
    lea dx, rm_msg
    int 21h
    call NewLine

    ; Вывод сообщения, что мы ожидаем нажатие клавиши. 
    mov ah, 09h
    lea dx, pm_msg_wait
    int 21h
    call NewLine

    ; Ожидание нажатия кнопки
    mov ah, 10h
    int 16h
    
    call ClearScreen

    xor eax, eax
	
	
    mov ax, code16
    shl eax, 4 ; Получаем линейный адрес                    
    mov word ptr gdt_code16.base_l, ax  
    shr eax, 16                       
    mov byte ptr gdt_code16.base_m, al  
    mov byte ptr gdt_code16.base_h, ah  

    mov ax, code32
    shl eax, 4                        
    mov word ptr gdt_code32.base_l, ax  
    shr eax, 16                       
    mov byte ptr gdt_code32.base_m, al  
    mov byte ptr gdt_code32.base_h, ah  

    mov ax, data32
    shl eax, 4                        
    mov word ptr gdt_data32.base_l, ax  
    shr eax, 16                       
    mov byte ptr gdt_data32.base_m, al  
    mov byte ptr gdt_data32.base_h, ah  

    mov ax, stack32
    shl eax, 4                        
    mov word ptr gdt_stack32.base_l, ax  
    shr eax, 16                       
    mov byte ptr gdt_stack32.base_m, al  
    mov byte ptr gdt_stack32.base_h, ah  

    mov ax, data32
    shl eax, 4

    add eax, offset gdt_null

    mov dword ptr pdescr+2, eax
    mov word ptr  pdescr, gdt_size-1
    lgdt fword ptr pdescr

    
    mov ax, code32
    mov es, ax

    lea eax, es:except_1
    mov idescr_0_12.offs_l, ax
    shr eax, 16
    mov idescr_0_12.offs_h, ax

    lea eax, es:except_13
    mov idescr_13.offs_l, ax 
    shr eax, 16             
    mov idescr_13.offs_h, ax 

    lea eax, es:except_1
    mov idescr_14_31.offs_l, ax 
    shr eax, 16             
    mov idescr_14_31.offs_h, ax 

    
    lea eax, es:new_int08
    mov int08.offs_l, ax
    shr eax, 16
    mov int08.offs_h, ax

    lea eax, es:new_int09
    mov int09.offs_l, ax 
    shr eax, 16             
    mov int09.offs_h, ax 

    ; Получаем линейный адрес IDT
    mov ax, data32
    shl eax, 4
    add eax, offset idt

    mov  dword ptr ipdescr + 2, eax 
    ; И размер IDT
    mov  word ptr  ipdescr, idt_size-1 
    
	 ; Сохранение масок
    in  al, 21h                     
    mov mask_master, al      
    in  al, 0A1h                    
    mov mask_slave, al 
    
	; Перепрограммирование контроллера прерываний
    mov al, 11h
    out 20h, al                     
    mov al, 32 
    out 21h, al                     
    mov al, 4
    out 21h, al
    mov al, 1
    out 21h, al

    mov al, 0FCh
    out 21h, al

    mov al, 0FFh
    out 0A1h, al
    
	; открытие линии A20
    in  al, 92h
    or  al, 2
    out 92h, al

    cli 
	
	;  lidt - load IDT - загрузить в регистр IDTR 
    ; Наш псевдодискриптор ipdescr, который содержит
    ; Лин адрес таблицы IDT и её размер.
    lidt fword ptr ipdescr
    
    ; Запрет немаскируемых прерываний. NMI
    mov al, 80h
    out 70h, al

    ; Переход в защищенный режим
    mov eax, cr0 ; Получаем содержимое регистра CR0
    or eax, 1    ; Установим бит защищённого режима
    mov cr0, eax ; Обновляем содержимое регистра CR0

    db  66h  ; Префикс изменения разрядности операнда (меняет на противоположный).
    db  0EAh ; Код команды far jmp.
    dd  offset pm_start ; Смещение
    dw  code32s         ; Сегмент


return_rm:
    ; возвращаем флаг pe
    mov eax, cr0
    and al, 0FEh                
    mov cr0, eax

    db  0EAh    
    dw  offset go
    dw  code16

go:
    ; обновляем все сегментные регистры
    mov ax, data32   
    mov ds, ax
    mov ax, code32
    mov es, ax
    mov ax, stack32   
    mov ss, ax
    mov ax, stack_size
    mov sp, ax
    
    ; возвращаем базовый вектор контроллера прерываний
    mov al, 11h
    out 20h, al
    mov al, 8
    out 21h, al
    mov al, 4
    out 21h, al
    mov al, 1
    out 21h, al

    ; восстанавливаем маски контроллеров прерываний
    mov al, mask_master
    out 21h, al
    mov al, mask_slave
    out 0A1h, al

    ; восстанавливаем вектор прерываний (на 1ый кб)
    lidt    fword ptr ipdescr16

    ; закрытие линии A20
    in  al, 70h 
    and al, 7Fh
    out 70h, al

    sti ; Резрешаем (аппаратные) прерывания     
    
    ; Очищаем экран
    call ClearScreen

    mov ah, 09h
    lea dx, pm_msg_out
    int 21h
    call NewLine

    ; Завершаем программу.
    mov ax, 4C00h
    int 21h

    code16_size = $-start  
code16 ends

end start