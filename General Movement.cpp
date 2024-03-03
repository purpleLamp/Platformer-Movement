using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.InputSystem;
using UnityEngine.SceneManagement;
using UnityEngine.Windows;
using static UnityEngine.GraphicsBuffer;

public class PlayerMovement : MonoBehaviour
{
    //General
    public Rigidbody2D rb;
    public LayerMask ground;
    bool facingRight = true;
    public SpriteRenderer spriteRenderer;
    private Color normalColor;

    //Movment
    public float moveSpeed;
    private float input;
    private float yinput;

    //Jump
    bool jumpRequest;
    float jumpTimer;
    public float jumpSpeed;
    float maxJumpTime = 0.1f;
    public float fallGravity;
    public float normalGravity;

    //Wall Sliding 
    private bool isWallSliding;
    public float wallSlidingSpeed = 2f;

    //Wall Jumping
    private bool isWallJumping;
    private float wallJumpingTimer;
    public Vector2 wallJumpingSpeed;
    bool specialFuckingCircumstance = false;

    //Dash
    public bool canDash = true;
    private bool isDashing;
    public float dashingPower;
    public float dashingTime;
    public float dashingCooldown;
    public TrailRenderer tr;

    private void OnDrawGizmos()
    {
        
        Debug.DrawRay(new Vector2(transform.position.x - 0.35f, transform.position.y - 1), new Vector2(0.7f,0), UnityEngine.Color.red);

        if (facingRight) Debug.DrawRay(new Vector2(transform.position.x + 0.7f, transform.position.y - 0.7f), new Vector2(0, 1.3f), UnityEngine.Color.red);
        else Debug.DrawRay(new Vector2(transform.position.x - 0.7f, transform.position.y - 0.7f), new Vector2(0,1.3f), UnityEngine.Color.red);
        
    }
    private void Start()
    {
        normalColor = spriteRenderer.color;
    }

    void Update()
    {
        if (isDashing) return;
        
        if (!isWallJumping)
        {
            rb.velocity = new Vector2(input * moveSpeed, rb.velocity.y);
        }
        else if (isWallJumping && !isGrounded())
        {
            rb.velocity = Vector2.Lerp(rb.velocity, (new Vector2(input * moveSpeed, rb.velocity.y)), 0.5f * Time.deltaTime);
        }

        if (jumpTimer > maxJumpTime)
        {
            jumpRequest = false;
        }

        if (rb.velocity.y < 0)
        {
            rb.gravityScale = fallGravity; //grav increases when falling 
        }
        else
        {
            rb.gravityScale = normalGravity;
        }
        wallStuff();

        //Mathf.Clamp(rb.velocity.y, -20, 20);
    }
    public void OnMove(InputAction.CallbackContext context)
    {
        input = context.ReadValue<Vector2>().x;
        yinput = context.ReadValue<Vector2>().y;

        if (input < 0) facingRight = false;
        else if (input > 0) facingRight = true;
    }

    public void Jump(InputAction.CallbackContext context)
    {

        if ((context.performed && isGrounded()) || (context.performed && wallCheck() && !isGrounded())) //When jump pressed and on ground
        {
            jumpRequest = true;
            jumpTimer = 0f;
        }

        if (context.canceled || jumpTimer >= maxJumpTime)
        {
            jumpRequest = false;
            isWallJumping = false;
            specialFuckingCircumstance = false;
        }
    }

    private void FixedUpdate()
    {
        if (isDashing) return;

        if (jumpRequest && !wallCheck() && !specialFuckingCircumstance)
        {
            rb.AddForce(Vector2.up * jumpSpeed, ForceMode2D.Impulse);
            jumpTimer += Time.deltaTime;
        }

        if (jumpRequest && specialFuckingCircumstance)
        {
            rb.AddForce(Vector2.up * jumpSpeed, ForceMode2D.Impulse);
            jumpTimer += Time.deltaTime;
        }
    }
    private bool isGrounded()
    {
        return Physics2D.Raycast(new Vector2(transform.position.x - 0.35f, transform.position.y - 1), Vector2.right, 0.7f, ground);
    }
    private bool wallCheck()
    {
        if (facingRight) return Physics2D.Raycast(new Vector2(transform.position.x + 0.7f, transform.position.y - 0.7f), Vector2.up, 1.3f, ground);
        else return Physics2D.Raycast(new Vector2(transform.position.x - 0.7f, transform.position.y - 0.7f), Vector2.up, 1.3f, ground);
    }

    private IEnumerator Dash()
    {
        canDash = false;
        isDashing = true;
        float originalGravity = rb.gravityScale;
        rb.gravityScale = 0f;
        rb.velocity = new Vector2(input * dashingPower, yinput * dashingPower);
        tr.emitting = true;
        spriteRenderer.color = Color.white;
        yield return new WaitForSeconds(dashingTime);
        tr.emitting = false;
        rb.gravityScale = originalGravity;
        isDashing = false;
     
    }

    public void DashHandler(InputAction.CallbackContext context)
    {
        if (context.performed && canDash && (input != 0 || yinput != 0))
        {
            StartCoroutine(Dash());
        }
    }

    private void wallStuff()
    {
        //Wall Sliding
        if (wallCheck() && input != 0f)
        {
            isWallSliding = true;
            rb.velocity = new Vector2(rb.velocity.x, Mathf.Clamp(rb.velocity.y, -wallSlidingSpeed, float.MaxValue));
        }
        else
        {
            isWallSliding = false;
        }

        //WallJumping
        if (jumpRequest && isWallSliding && !isGrounded() && !specialFuckingCircumstance)
        {
            if (!facingRight)
            {
                isWallJumping = true;
                rb.velocity = new Vector2(wallJumpingSpeed.x, wallJumpingSpeed.y);
                jumpTimer += Time.deltaTime;
            }
            else
            {
                isWallJumping = true;
                rb.velocity = new Vector2(-wallJumpingSpeed.x, wallJumpingSpeed.y);
                jumpTimer += Time.deltaTime;
            }
        }

        //Other Checks
        if (isGrounded())
        {
            isWallJumping = false;
            canDash = true;
            spriteRenderer.color = normalColor;
        }
        if (isGrounded() && wallCheck())
        {
            specialFuckingCircumstance = true;
        }
        if (!wallCheck() || !jumpRequest || isWallJumping)
        {
            specialFuckingCircumstance = false;
        }
    }

}
