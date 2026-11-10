/*
 * ================================================================================
 * File: modules/containers/src/orbitc_stack.c
 * ================================================================================
 */
#include "orbitc_stack.h"

#if (ENABLE_CONTAINER_STACK == 1)

#define ORBITC_STACK_MAGIC ((ui32_t)0x5354434BUL) /* "STCK" */

static inline void orbitc_stack_copy(byte_t *dst, const byte_t *src, usize_t len)
{
    usize_t i;
    for (i = 0U; i < len; ++i)
    {
        dst[i] = src[i];
    }
}

status_t orbitc_stack_init(
    stack_t *stack,
    void *buf,
    usize_t elem_size,
    usize_t capacity)
{
    status_t st = STATUS_OK;
    bool_t   parameters_valid = ORBITC_FALSE;

    orbitc_assert_not_null(
        stack,
        &st,
        STATUS_MOD_CONTAINERS,
        (ui8_t)CONTAINERS_CODE_NULL_PTR);

    orbitc_assert_not_null(
        buf,
        &st,
        STATUS_MOD_CONTAINERS,
        (ui8_t)CONTAINERS_CODE_NULL_PTR);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        if ((elem_size != 0U) &&
            (capacity != 0U) &&
            (capacity <= (ORBITC_USIZE_MAX / elem_size)))
        {
            parameters_valid = ORBITC_TRUE;
        }

        orbitc_assert_param(
            parameters_valid,
            &st,
            STATUS_SEV_ERROR,
            STATUS_MOD_CONTAINERS,
            (ui8_t)CONTAINERS_CODE_INVALID_PARAM);
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        stack->buf        = (byte_t *)buf;
        stack->elem_size  = elem_size;
        stack->capacity   = capacity;
        stack->top        = 0U;
        stack->high_water = 0U;
        stack->init_magic = ORBITC_STACK_MAGIC;
    }

    return st;
}

status_t orbitc_stack_reset(stack_t *stack)
{
    status_t st = STATUS_OK;

    orbitc_assert_not_null(stack, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        if (stack->init_magic != ORBITC_STACK_MAGIC)
        {
            orbitc_status_latch(
                &st,
                STATUS_MAKE(
                    STATUS_SEV_ERROR,
                    STATUS_MOD_CONTAINERS,
                    STATUS_CLS_INVALID_PARAM,
                    (ui8_t)CONTAINERS_CODE_NOT_INIT));
        }
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        stack->top = 0U;
    }
    
    return st;
}

status_t orbitc_stack_push(stack_t *stack, const void *elem)
{
    status_t st = STATUS_OK;
    byte_t  *dst;

    orbitc_assert_not_null(stack, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);
    orbitc_assert_not_null(elem, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        if (stack->init_magic != ORBITC_STACK_MAGIC)
        {
            orbitc_status_latch(
                &st,
                STATUS_MAKE(
                    STATUS_SEV_ERROR,
                    STATUS_MOD_CONTAINERS,
                    STATUS_CLS_INVALID_PARAM,
                    (ui8_t)CONTAINERS_CODE_NOT_INIT));
        }
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        if (stack->top == stack->capacity)
        {
            orbitc_status_latch(
                &st,
                STATUS_MAKE(
                    STATUS_SEV_ERROR,
                    STATUS_MOD_CONTAINERS,
                    STATUS_CLS_CAPACITY,
                    (ui8_t)CONTAINERS_CODE_FULL));
        }
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        dst = stack->buf + (stack->top * stack->elem_size);
        orbitc_stack_copy(dst, (const byte_t *)elem, stack->elem_size);

        stack->top += 1U;
        if (stack->top > stack->high_water)
        {
            stack->high_water = stack->top;
        }
    }

    return st;
}

status_t orbitc_stack_pop(stack_t *stack, void *out_elem)
{
    status_t      st = STATUS_OK;
    const byte_t *src;

    orbitc_assert_not_null(stack, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);
    orbitc_assert_not_null(out_elem, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        if (stack->init_magic != ORBITC_STACK_MAGIC)
        {
            orbitc_status_latch(
                &st,
                STATUS_MAKE(
                    STATUS_SEV_ERROR,
                    STATUS_MOD_CONTAINERS,
                    STATUS_CLS_INVALID_PARAM,
                    (ui8_t)CONTAINERS_CODE_NOT_INIT));
        }
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        if (stack->top == 0U)
        {
            orbitc_status_latch(
                &st,
                STATUS_MAKE(
                    STATUS_SEV_ERROR,
                    STATUS_MOD_CONTAINERS,
                    STATUS_CLS_NOT_FOUND,
                    (ui8_t)CONTAINERS_CODE_EMPTY));
        }
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        stack->top -= 1U;
        src = stack->buf + (stack->top * stack->elem_size);
        orbitc_stack_copy((byte_t *)out_elem, src, stack->elem_size);
    }
    
    return st;
}

status_t orbitc_stack_peek(const stack_t *stack, void *out_elem)
{
    status_t      st = STATUS_OK;
    const byte_t *src;

    orbitc_assert_not_null(stack, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);
    orbitc_assert_not_null(out_elem, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        if (stack->init_magic != ORBITC_STACK_MAGIC)
        {
            orbitc_status_latch(
                &st,
                STATUS_MAKE(
                    STATUS_SEV_ERROR,
                    STATUS_MOD_CONTAINERS,
                    STATUS_CLS_INVALID_PARAM,
                    (ui8_t)CONTAINERS_CODE_NOT_INIT));
        }
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        if (stack->top == 0U)
        {
            orbitc_status_latch(
                &st,
                STATUS_MAKE(
                    STATUS_SEV_ERROR,
                    STATUS_MOD_CONTAINERS,
                    STATUS_CLS_NOT_FOUND,
                    (ui8_t)CONTAINERS_CODE_EMPTY));
        }
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        src = stack->buf + ((stack->top - 1U) * stack->elem_size);
        orbitc_stack_copy((byte_t *)out_elem, src, stack->elem_size);
    }
    
    return st;
}

status_t orbitc_stack_is_empty(const stack_t *stack, bool_t *out_empty)
{
    status_t st = STATUS_OK;
    
    orbitc_assert_not_null(stack, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);
    orbitc_assert_not_null(out_empty, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        if (stack->init_magic != ORBITC_STACK_MAGIC)
        {
            orbitc_status_latch(
                &st,
                STATUS_MAKE(
                    STATUS_SEV_ERROR,
                    STATUS_MOD_CONTAINERS,
                    STATUS_CLS_INVALID_PARAM,
                    (ui8_t)CONTAINERS_CODE_NOT_INIT));
        }
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        *out_empty = (stack->top == 0U) ? ORBITC_TRUE : ORBITC_FALSE;
    }
    
    return st;
}

status_t orbitc_stack_is_full(const stack_t *stack, bool_t *out_full)
{
    status_t st = STATUS_OK;

    orbitc_assert_not_null(stack, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);
    orbitc_assert_not_null(out_full, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        if (stack->init_magic != ORBITC_STACK_MAGIC)
        {
            orbitc_status_latch(
                &st,
                STATUS_MAKE(
                    STATUS_SEV_ERROR,
                    STATUS_MOD_CONTAINERS,
                    STATUS_CLS_INVALID_PARAM,
                    (ui8_t)CONTAINERS_CODE_NOT_INIT));
        }
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        *out_full = (stack->top == stack->capacity) ? ORBITC_TRUE : ORBITC_FALSE;
    }
    
    return st;
}

status_t orbitc_stack_get_count(const stack_t *stack, usize_t *out_count)
{
    status_t st = STATUS_OK;
    
    orbitc_assert_not_null(stack, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);
    orbitc_assert_not_null(out_count, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        if (stack->init_magic != ORBITC_STACK_MAGIC)
        {
            orbitc_status_latch(
                &st,
                STATUS_MAKE(
                    STATUS_SEV_ERROR,
                    STATUS_MOD_CONTAINERS,
                    STATUS_CLS_INVALID_PARAM,
                    (ui8_t)CONTAINERS_CODE_NOT_INIT));
        }
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        *out_count = stack->top;
    }
    
    return st;
}

#else
typedef int orbitc_stack_disabled_tu_t;
#endif /* ENABLE_CONTAINER_STACK == 1 */