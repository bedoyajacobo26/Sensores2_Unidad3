using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.SceneManagement;

public class CONFIGURACION : MonoBehaviour
{
    public InputField Puerto_esp32;
    public InputField ID_esp32;

    public string vPuerto_esp32;
    public string vID_esp32;

    // Start is called before the first frame update
    void Start()
    {

    }

    // Update is called once per frame
    void Update()
    {
        vPuerto_esp32 = Puerto_esp32.text;
        vID_esp32 = ID_esp32.text;

    }
    void Awake()
    {
        GameObject[] DontDestroy = GameObject.FindGameObjectsWithTag("DD");

        if (DontDestroy.Length > 1)
        {
            Destroy(this.gameObject);
        }

        DontDestroyOnLoad(this.gameObject);
    }

    public void Jugar()
    {
        SceneManager.LoadScene("SampleScene");
    }
}
